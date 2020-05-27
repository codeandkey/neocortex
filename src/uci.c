#include "uci.h"
#include "position.h"
#include "move.h"
#include "util.h"
#include "search.h"
#include "perft.h"
#include "searchctl.h"

#include <string.h>

static int _nc_uci_try_arg(char** args, int i, int num_args, int def);

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
	nc_searchctl_position(game_pos);

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

		if (!strcmp(command, "stop")) {
			nc_searchctl_stop();
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
					nc_move inpmove = nc_move_fromstr(movestr);

					if (inpmove == NC_MOVE_NULL) {
						nc_abort("Couldn't parse input move '%s'!", movestr);
					}

					if (!nc_position_make_move(&game_pos, inpmove)) {
						nc_abort("Position reported illegal move '%s'!", nc_move_tostr(inpmove));
					}
				}
			}

			nc_searchctl_position(game_pos);
		}

		if (!strcmp(command, "go")) {
			/* Collect opts */
			nc_searchopts opts = {0};

			char* args[32];
			int num_args = 0;

			while ((args[num_args] = strtok_r(NULL, " ", &saveptr))) {
				if (++num_args >= 32) {
					nc_error("Possible argument overflow in go command");
					break;
				}
			}

			for (int i = 0; i < num_args; ++i) {
				if (!strcmp(args[i], "wtime")) {
					opts.wtime = _nc_uci_try_arg(args, i, num_args, 0);
					continue;
				}

				if (!strcmp(args[i], "btime")) {
					opts.btime = _nc_uci_try_arg(args, i, num_args, 0);
					continue;
				}

				if (!strcmp(args[i], "winc")) {
					opts.winc = _nc_uci_try_arg(args, i, num_args, 0);
					continue;
				}

				if (!strcmp(args[i], "binc")) {
					opts.binc = _nc_uci_try_arg(args, i, num_args, 0);
					continue;
				}

				if (!strcmp(args[i], "movestogo")) {
					opts.movestogo = _nc_uci_try_arg(args, i, num_args, 0);
					continue;
				}

				if (!strcmp(args[i], "depth")) {
					opts.depth = _nc_uci_try_arg(args, i, num_args, 0);
					continue;
				}

				if (!strcmp(args[i], "nodes")) {
					opts.nodes = _nc_uci_try_arg(args, i, num_args, 0);
					continue;
				}

				if (!strcmp(args[i], "mate")) {
					opts.mate = _nc_uci_try_arg(args, i, num_args, 0);
					continue;
				}

				if (!strcmp(args[i], "movetime")) {
					opts.movetime = _nc_uci_try_arg(args, i, num_args, 0);
					continue;
				}

				if (!strcmp(args[i], "infinite")) {
					opts.infinite = 1;
					continue;
				}
			}

			nc_searchctl_go(&opts);
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

int _nc_uci_try_arg(char** args, int i, int num_args, int def) {
	if (i + 1 >= num_args) {
		nc_error("%s: expected argument", args[i]);
		return def;
	}

	char* endptr = NULL;

	int res = (int) strtol(args[i + 1], &endptr, 10);

	if (*endptr != '\0') {
		nc_error("%s: invalid argument \"%s\"", args[i], args[i + 1]);
		return def;
	}

	return res;
}
