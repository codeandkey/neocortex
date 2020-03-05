#include "uci.h"
#include "position.h"
#include "move.h"
#include "util.h"
#include "search.h"
#include "perft.h"

#include <string.h>

int nc_uci_start(FILE* in, FILE* out) {
    char uci_buf[NC_UCI_BUFLEN];

    /* disable output buffering */
    setvbuf(out, NULL, _IONBF, 0);

    /* wait for uci */
    if (!fgets(uci_buf, sizeof uci_buf, in)) {
        nc_error("UCI input stream error!");
        return -1;
    }

    /* strip newline from command buffer */
    if (uci_buf[strlen(uci_buf) - 1] == '\n') {
        uci_buf[strlen(uci_buf) - 1] = '\0';
    }

    if (strcmp(uci_buf, "uci")) {
        nc_error("Expected 'uci', read '%s'", uci_buf);
        return -1;
    }

    fprintf(out, "id name " NC_UCI_NAME "\n");
    fprintf(out, "id author " NC_UCI_AUTHOR "\n");
    fprintf(out, "uciok\n");

    /* Initialize a game position. */
    nc_position game_pos;
    nc_position_init(&game_pos);
    nc_eval lastscore = NC_EVAL_MIN;

    /* Wait for commands! */
    while (fgets(uci_buf, sizeof uci_buf, in)) {
        /* strip newline from command buffer */
        if (uci_buf[strlen(uci_buf) - 1] == '\n') {
            uci_buf[strlen(uci_buf) - 1] = '\0';
        }

        char* saveptr = NULL;
        char* command = strtok_r(uci_buf, " \n", &saveptr);

        if (!command) continue;

        if (!strcmp(command, "dump")) {
            nc_position_dump(&game_pos, stderr, 1);
        }

        if (!strcmp(command, "quit")) {
            return 0;
        }

        if (!strcmp(command, "isready")) {
            fprintf(out, "readyok\n");
        }

        if (!strcmp(command, "position")) {
            char* next = strtok_r(NULL, " \n", &saveptr);

            if (!strcmp(next, "startpos")) {
                nc_position_init(&game_pos);
            } else {
                /* Build a FENbuf */
                char fen[256] = {0};
                int ind = 0;

                memcpy(fen + ind, next, strlen(next));
                ind += strlen(next);
                fen[ind++] = ' ';

                for (int i = 0; i < 5; ++i) {
                    next = strtok_r(NULL, " \n", &saveptr);
                    nc_assert(next);
                    memcpy(fen + ind, next, strlen(next));
                    ind += strlen(next);
                    fen[ind++] = ' ';
                };

                nc_position_init_fen(&game_pos, fen);
            }

            char* moves_word = strtok_r(NULL, " \n", &saveptr);

            if (moves_word) {
                if (strcmp(moves_word, "moves")) {
                    nc_abort("Expected 'moves', read '%s'", moves_word);
                }

                for (char* movestr = strtok_r(NULL, " \n", &saveptr); movestr; movestr = strtok_r(NULL, " \n", &saveptr)) {
                    nc_movelist next_moves;
                    nc_movelist_clear(&next_moves);
                    nc_position_legal_moves(&game_pos, &next_moves);

                    nc_move inpmove = nc_move_fromstr(movestr);

                    if (inpmove == NC_MOVE_NULL) {
                        nc_abort("Couldn't parse input move '%s'!", movestr);
                    }

                    nc_move matched = nc_movelist_match(&next_moves, inpmove);

                    if (matched == NC_MOVE_NULL) {
                        nc_abort("Couldn't match input move '%s'!", nc_move_tostr(inpmove));
                    }

                    nc_position_make_move(&game_pos, matched);
                }
            }
        }

        if (!strcmp(command, "go")) {
            int movetime[2] = {0};
            int forcedepth = 0;

            for (char* arg = strtok_r(NULL, " ", &saveptr); arg; arg = strtok_r(NULL, " ", &saveptr)) {
                if (!strcmp(arg, "wtime")) {
                    movetime[NC_WHITE] = strtol(strtok_r(NULL, " ", &saveptr), NULL, 10) / NC_UCI_MOVETIME_DIV;
                }

                if (!strcmp(arg, "btime")) {
                    movetime[NC_BLACK] = strtol(strtok_r(NULL, " ", &saveptr), NULL, 10) / NC_UCI_MOVETIME_DIV;
                }

                if (!strcmp(arg, "forcedepth")) {
                    forcedepth = strtol(strtok_r(NULL, " ", &saveptr), NULL, 10);
                }
            }

            int ourtime = movetime[game_pos.color_to_move];
            if (ourtime > NC_UCI_MAX_MOVETIME) ourtime = NC_UCI_MAX_MOVETIME; /* don't wait too long */

            nc_timepoint maxtime = (ourtime && !forcedepth) ? nc_timer_futurems(ourtime) : 0;
            nc_timepoint starttime = nc_timer_current();

            nc_movelist best_pv = {0};
            nc_movelist_clear(&best_pv);
            nc_eval complete_score = NC_EVAL_MIN;

            for (int d = 1; d <= NC_UCI_MAXDEPTH; ++d) {
                if (forcedepth && d > forcedepth) break;
                int elapsed = nc_timer_elapsed_ms(starttime);
                nc_eval early_score = (ourtime - elapsed) / NC_UCI_ACCEPTABLE_SCORE_FRACTION;

                if (!forcedepth && d > 1) {
                    /* Check for time control abort. */
                    if (elapsed >= ourtime || elapsed >= (ourtime * NC_UCI_TIME_FACTOR) / 100) {
                        break;
                    }
                }

                nc_movelist current_pv;
                nc_eval score = nc_search(&game_pos, d, &current_pv, maxtime);

                int incomplete = 0;
                if (!forcedepth && (nc_timer_current() >= maxtime && d > 1)) incomplete = 1;

                fprintf(out, "info depth %d%s nodes %d nps %d time %d score %s pv", d, incomplete ? "-" : "", nc_search_get_nodes(), nc_search_get_nps(), nc_search_get_time(), nc_eval_tostr(score));

                for (int i = 0; i < current_pv.len; ++i) {
                    fprintf(out, " %s", nc_move_tostr(current_pv.moves[i]));
                }

                fprintf(out, "\n");

                if (incomplete) {
                    break;
                } else {
                    best_pv = current_pv;
                    complete_score = score;

                    if (!forcedepth && score < lastscore && lastscore - score >= NC_UCI_BLUNDER) {
                        int nextitertime = ((ourtime - elapsed) * NC_UCI_BLUNDER_REQTIME) / 100;
                        if (nc_search_get_time() * NC_UCI_EBF <= nextitertime) {
                            /* Move would be a blunder.. try a higher depth if we think we have time */
                            ourtime = nextitertime;
                            maxtime = nc_timer_futurems(nextitertime);
                            starttime = nc_timer_current();
                            continue;
                        }
                    }
                }

                if (!forcedepth) {
                    if (elapsed + nc_search_get_time() * NC_UCI_EBF >= (ourtime * NC_UCI_TIME_FACTOR) / 100) {
                        break;
                    }

                    if (score >= early_score && d >= NC_UCI_ACCEPTABLE_SCORE_FRACTION_MIN_DEPTH) break;
                    if (nc_search_was_only_move()) break;
                }
            }

            fprintf(out, "bestmove %s\n", nc_move_tostr(best_pv.moves[0]));
            lastscore = complete_score;
        }

        if (!strcmp(command, "perft")) {
            char* depthstr = strtok_r(NULL, " ", &saveptr);

            if (!depthstr) {
                continue;
            }

            int depth = strtol(depthstr, NULL, 10);

            nc_perft_run(out, &game_pos, depth);
        }
    }

    return 0;
}
