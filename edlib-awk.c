#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <gawkapi.h>
#include "edlib.h"

int plugin_is_GPL_compatible;

static const gawk_api_t *api;

static awk_ext_id_t ext_id;
static const char *ext_version = NULL; /* or … = "some string" */

static awk_bool_t (*init_func)(void) = NULL;

static awk_value_t *do_edlib(int nargs, awk_value_t *result, struct awk_ext_func *finfo) {
    awk_value_t val1;
    awk_value_t val2;
    awk_string_t str1;
    awk_string_t str2;

    awk_value_t mode_val;
    char *mode_str;
    EdlibAlignMode mode = EDLIB_MODE_NW;

    awk_value_t max_dist_val;
    int max_dist = -1;

    if (get_argument(0, AWK_STRING, &val1) &&
        get_argument(1, AWK_STRING, &val2)) {
        str1 = val1.str_value;
        str2 = val2.str_value;
    } else if (do_lint) {
        lintwarn(ext_id, "edlib_hw: called with innapropriate arguments");
    }

    if (nargs > 2) {
        if (get_argument(2, AWK_STRING, &mode_val)) {
            mode_str = mode_val.str_value.str;
        } else {
            lintwarn(ext_id, "edlib: supplied mode is not a string, defaulting to \"NW\"");
        }

        if (!strcmp(mode_str, "NW")) {
            mode = EDLIB_MODE_NW;
        } else if (!strcmp(mode_str, "SHW")) {
            mode = EDLIB_MODE_SHW;
        } else if (!strcmp(mode_str, "HW")) {
            mode = EDLIB_MODE_HW;
        } else {
            lintwarn(ext_id, "edlib: supplied mode is invalid, defaulting to \"NW\"");
        }
    } else {
        mode = EDLIB_MODE_NW;
    }

    if (nargs > 3) {
        if (get_argument(3, AWK_NUMBER, &max_dist_val) &&
            max_dist_val.num_type == AWK_NUMBER_TYPE_DOUBLE) {
            max_dist = max_dist_val.num_value;
        } else {
            lintwarn(ext_id, "edlib: supplied maximum distance is not a simple number, defaulting to unlimited (-1)");
        }
    }

    EdlibAlignConfig config =
        edlibNewAlignConfig(max_dist, mode, EDLIB_TASK_PATH, NULL, 0);

    EdlibAlignResult alignResult =
        edlibAlign(str1.str, str1.len, str2.str, str2.len, config);

    char *cigar =
        edlibAlignmentToCigar(alignResult.alignment, alignResult.alignmentLength, EDLIB_CIGAR_STANDARD);

    make_malloced_string(cigar, strlen(cigar), result);

    edlibFreeAlignResult(alignResult);

    return result;
}

static awk_ext_func_t func_table[] = {
    { "edlib", do_edlib, 4, 2, awk_false, NULL },
};

dl_load_func(func_table, edlib, "")
