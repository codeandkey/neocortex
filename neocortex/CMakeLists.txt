add_executable (neocortex neocortex.c attacks.c board.c zobrist.c position.c types.c search.c uci.c mcts.c)
target_link_libraries(neocortex m)
