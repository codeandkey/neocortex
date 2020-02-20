#pragma once

/*
 * Move type.
 *
 * bits [0, 5]: destination square
 * bits [6, 10]: source square
 * bits [11, 13]: promote type
 */

typedef int move_t;

#define NC3_MOVE(from, to) (from << 6 | to)

#define NC3_PROMOTE 0xF000
