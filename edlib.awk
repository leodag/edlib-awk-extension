@load "edlib"

function _repeat_times(string, times,     out) {
    for (i = 0; i < times; i++) {
        out = out string;
    }
    return out;
}

function edlib_print_nice_alignment(align_result, query, target,     nice_align) {
    edlib_get_nice_alignment(nice_align, align_result, query, target);

    print("T:  " nice_align["target_aligned"] \
        " (" align_result["start_locations"][0] " - "  align_result["end_locations"][0] ")");
    print("    " nice_align["matched_aligned"]);
    print("Q:  " nice_align["query_aligned"] " (0 - " length(query) - 1 ")");
}

function edlib_get_nice_alignment(nice_align, align_result, query, target, gap_symbol,
                            # locals
                            cigar, fields, num_occurrences, alignment_operation,
                            query_pos, target_pos, query_aln, target_aln, matched_aln,
                            save_sorted) {
    if (gap_symbol == "") {
        gap_symbol = "-";
    }

    cigar = align_result["cigar"];
    patsplit(cigar, fields, "[[:digit:]]+[IDM=X]");

    query_pos = 1;
    target_pos = align_result["start_locations"][0] + 1;
    query_aln = "";
    matched_aln = "";
    target_aln = "";

    for (i in fields) {
        num_occurrences = substr(fields[i], 0, length(fields[i]) - 1);
        alignment_operation = substr(fields[i], length(fields[i]), 1);

        switch (alignment_operation) {
            case "=":
                target_aln = target_aln substr(target, target_pos, num_occurrences);
                target_pos += num_occurrences;
                query_aln = query_aln substr(query, query_pos, num_occurrences);
                query_pos += num_occurrences;
                match_aln = match_aln _repeat_times("|", num_occurrences);
                break;
            case "X":
                target_aln = target_aln substr(target, target_pos, num_occurrences);
                target_pos += num_occurrences;
                query_aln = query_aln substr(query, query_pos, num_occurrences);
                query_pos += num_occurrences;
                match_aln = match_aln _repeat_times(".", num_occurrences);
                break;
            case "D":
                target_aln = target_aln substr(target, target_pos, num_occurrences);
                target_pos += num_occurrences;
                query_aln = query_aln _repeat_times(gap_symbol, num_occurrences);
                match_aln = match_aln _repeat_times(gap_symbol, num_occurrences);
                break;
            case "I":
                target_aln = target_aln _repeat_times(gap_symbol, num_occurrences);
                query_aln = query_aln substr(query, query_pos, num_occurrences);
                query_pos += num_occurrences;
                match_aln = match_aln _repeat_times(gap_symbol, num_occurrences);
                break;
        }
    }

    delete nice_align;
    nice_align["target_aligned"] = target_aln;
    nice_align["matched_aligned"] = match_aln;
    nice_align["query_aligned"] = query_aln;
}
