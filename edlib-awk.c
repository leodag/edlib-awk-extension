#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "gawkapi.h"
#include "edlib.h"

int plugin_is_GPL_compatible;

static const gawk_api_t *api;

static awk_ext_id_t ext_id;
static const char *ext_version = "edlib extension: version 0.9.1";

static awk_bool_t (*init_func)(void) = NULL;

// Convenience method, throws a fatal error on failure and receives a string index
static void really_set_array_element(awk_array_t array, char *index_str, awk_value_t *value) {
    awk_value_t index;
    make_const_string(index_str, strlen(index_str), &index);

    if (!set_array_element(array, &index, value)) {
        fatal(ext_id, "edlib: set_array_element failed");
    }

    return;
}

static void edlib_result_into_array(EdlibAlignResult align_result, char *cigar_str, awk_array_t array) {
    int i;
    awk_value_t index;
    awk_value_t value;
    awk_array_t end_locations_array;
    awk_array_t start_locations_array;

    make_malloced_string(cigar_str, strlen(cigar_str), &value);
    really_set_array_element(array, "cigar", &value);

    make_number(align_result.editDistance, &value);
    really_set_array_element(array, "edit_distance", &value);

    make_number(align_result.alphabetLength, &value);
    really_set_array_element(array, "alphabet_length", &value);

    end_locations_array = create_array();
    value.val_type = AWK_ARRAY;
    value.array_cookie = end_locations_array;
    really_set_array_element(array, "end_locations", &value);
    end_locations_array = value.array_cookie;

    start_locations_array = create_array();
    value.val_type = AWK_ARRAY;
    value.array_cookie = start_locations_array;
    really_set_array_element(array, "start_locations", &value);
    start_locations_array = value.array_cookie;

    for (i = 0; i < align_result.numLocations; i++) {
        make_number(i, &index);
        make_number(align_result.endLocations[i], &value);
        if (!set_array_element(end_locations_array, &index, &value)) {
            fatal(ext_id, "edlib: set_array_element failed");
        }
    }

    if (align_result.startLocations != NULL) {
        for (i = 0; i < align_result.numLocations; i++) {
            make_number(i, &index);
            make_number(align_result.startLocations[i], &value);
            if (!set_array_element(start_locations_array, &index, &value)) {
                fatal(ext_id, "edlib: set_array_element failed");
            }
        }
    }

    return;
}

static awk_value_t *do_edlib(int nargs, awk_value_t *return_val, struct awk_ext_func *finfo) {
    awk_value_t result_val;
    awk_array_t result_array;

    awk_value_t val1;
    awk_value_t val2;
    awk_string_t str1;
    awk_string_t str2;

    awk_value_t align_mode_val;
    char *align_mode_str;
    EdlibAlignMode mode = EDLIB_MODE_NW;

    awk_value_t cigar_mode_val;
    char *cigar_mode_str;
    EdlibCigarFormat cigar_mode = EDLIB_CIGAR_EXTENDED;

    awk_value_t max_dist_val;
    int max_dist = -1;

    if (get_argument(0, AWK_ARRAY, &result_val)) {
        result_array = result_val.array_cookie;
        if (! clear_array(result_array)) {
            update_ERRNO_int(ENOMEM);
            fatal(ext_id, "edlib: clear_array failed");
        }
    } else {
        fatal(ext_id, "edlib: first argument is not an array");
    }

    if (get_argument(1, AWK_STRING, &val1) &&
        get_argument(2, AWK_STRING, &val2)) {
        str1 = val1.str_value;
        str2 = val2.str_value;
    } else {
        fatal(ext_id, "edlib: called with innapropriate values for alignment");
    }

    if (nargs > 3) {
        if (get_argument(3, AWK_STRING, &align_mode_val)) {
            align_mode_str = align_mode_val.str_value.str;
        } else {
            fatal(ext_id, "edlib: supplied mode is not a string");
        }

        if (strcmp(align_mode_str, "NW") == 0) {
            mode = EDLIB_MODE_NW;
        } else if (strcmp(align_mode_str, "SHW") == 0) {
            mode = EDLIB_MODE_SHW;
        } else if (strcmp(align_mode_str, "HW") == 0) {
            mode = EDLIB_MODE_HW;
        } else {
            fatal(ext_id, "edlib: supplied alignment mode is invalid");
        }
    } else {
        mode = EDLIB_MODE_NW;
    }

    if (nargs > 4) {
        if (get_argument(4, AWK_STRING, &cigar_mode_val)) {
            cigar_mode_str = cigar_mode_val.str_value.str;
        } else {
            fatal(ext_id, "edlib: supplied cigar mode is not a string");
        }

        if (strcmp(cigar_mode_str, "STANDARD") == 0) {
            cigar_mode = EDLIB_CIGAR_STANDARD;
        } else if (strcmp(cigar_mode_str, "EXTENDED") == 0) {
            cigar_mode = EDLIB_CIGAR_EXTENDED;
        } else {
            fatal(ext_id, "edlib: supplied cigar mode is invalid");
        }
    } else {
        cigar_mode = EDLIB_CIGAR_EXTENDED;
    }

    if (nargs > 5) {
        if (get_argument(5, AWK_NUMBER, &max_dist_val) &&
            max_dist_val.num_type == AWK_NUMBER_TYPE_DOUBLE) {
            max_dist = max_dist_val.num_value;
        } else {
            fatal(ext_id, "edlib: supplied maximum distance is not a simple number");
        }
    }

    EdlibAlignConfig config =
        edlibNewAlignConfig(max_dist, mode, EDLIB_TASK_PATH, NULL, 0);

    EdlibAlignResult align_result =
        edlibAlign(str1.str, str1.len, str2.str, str2.len, config);

    if (align_result.status != EDLIB_STATUS_OK) {
        edlibFreeAlignResult(align_result);

        nonfatal(ext_id, "edlib: an unknown error occured during alignment");
        make_number(0, return_val);

        return return_val;
    }

    char *cigar_str =
        edlibAlignmentToCigar(align_result.alignment, align_result.alignmentLength, cigar_mode);

    edlib_result_into_array(align_result, cigar_str, result_array);

    edlibFreeAlignResult(align_result);

    make_number(1, return_val);
    return return_val;
}

static awk_ext_func_t func_table[] = {
    { "edlib", do_edlib, 6, 3, awk_false, NULL },
};

dl_load_func(func_table, edlib, "")
