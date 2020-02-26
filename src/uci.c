#include "uci.h"
#include "position.h"
#include "move.h"
#include "util.h"
#include "search.h"

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

    /* Wait for commands! */
    while (fgets(uci_buf, sizeof uci_buf, in)) {
        /* strip newline from command buffer */
        if (uci_buf[strlen(uci_buf) - 1] == '\n') {
            uci_buf[strlen(uci_buf) - 1] = '\0';
        }

        char* command = strtok(uci_buf, " \n");

        if (!strcmp(command, "quit")) {
            return 0;
        }

        if (!strcmp(command, "isready")) {
            fprintf(out, "readyok\n");
        }

        if (!strcmp(command, "position")) {
            char* next = strtok(NULL, " \n");

            if (strcmp(next, "startpos")) {
                nc_abort("FEN parsing not implemented!");
            }

            nc_position_init(&game_pos);

            char* moves_word = strtok(NULL, " \n");

            if (moves_word) {
                if (strcmp(moves_word, "moves")) {
                    nc_abort("Expected 'moves', read '%s'", moves_word);
                }

                for (char* movestr = strtok(NULL, " \n"); movestr; movestr = strtok(NULL, " \n")) {
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

            for (char* arg = strtok(NULL, " "); arg; arg = strtok(NULL, " ")) {
                if (!strcmp(arg, "wtime")) {
                    movetime[NC_WHITE] = strtol(strtok(NULL, " "), NULL, 10) / 3;
                }

                if (!strcmp(arg, "btime")) {
                    movetime[NC_BLACK] = strtol(strtok(NULL, " "), NULL, 10) / 3;
                }
            }

            int maxtime = movetime[game_pos.color_to_move] ? nc_timer_futurems(movetime[game_pos.color_to_move]) : 0;

            nc_move pv[NC_UCI_MAXDEPTH];
            pv[0] = NC_MOVE_NULL;

            for (int d = 1; d <= NC_UCI_MAXDEPTH; ++d) {
                nc_eval score = nc_search(&game_pos, d, pv, maxtime);

                fprintf(out, "info depth %d nodes %d nps %d time %d score %s pv %s\n", d, nc_search_get_nodes(), nc_search_get_nps(), nc_search_get_time(), nc_eval_tostr(score), nc_move_tostr(pv[0]));

                if (maxtime && nc_timer_current() >= maxtime) break;
            }

            fprintf(out, "bestmove %s\n", nc_move_tostr(pv[0]));
        }
    }

    return 0;
}
