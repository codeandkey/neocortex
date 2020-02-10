#pragma once

#include <cstdint>
#include <string>

namespace nc2 {
    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;

    /**
     * Gets a printable form of a bitboard.
     * Contains newlines after every rank.
     *
     * @param bb Input bitboard.
     * @return Printable bitboard string.
     */
    std::string bitboard_to_string(u64 bb);
}
