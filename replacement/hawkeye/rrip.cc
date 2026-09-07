#include "rrip.h"

#include <algorithm>
#include <stdexcept>

namespace {

// Hawkeye uses 3-bit RRPVs.
//
// Therefore:
//     minimum RRPV = 0
//     maximum RRPV = 7
//
constexpr int MAX_RRPV = 7;

// Cache-friendly lines should never be aged to 7.
//
// When aging, Hawkeye increments values only if RRPV < 6.
constexpr int MAX_FRIENDLY_RRPV = 6;

} // namespace

void update_rrpv(std::vector<int>& rrpv,
                 std::size_t way,
                 Classification cls,
                 bool is_hit)
{
    if (way >= rrpv.size()) {
        throw std::out_of_range("way is outside the RRPV vector");
    }

    switch (cls) {

    case Classification::CACHE_AVERSE:
        // Table 1:
        //
        // Cache-averse + hit  -> RRPV = 7
        // Cache-averse + miss -> RRPV = 7
        //
        // Therefore hits do NOT promote an averse line.
        rrpv[way] = MAX_RRPV;
        break;

    case Classification::CACHE_FRIENDLY:
        // Table 1:
        //
        // Cache-friendly + hit -> RRPV = 0
        // Cache-friendly + miss -> RRPV = 0
        //
        // On a miss, age the other cache-friendly lines.
        if (!is_hit) {

            for (std::size_t i = 0; i < rrpv.size(); ++i) {

                // Do not age lines that are already at 7.
                //
                // In Hawkeye, RRPV=7 represents cache-averse
                // lines and they remain immediately evictable.
                //
                // Friendly lines are aged only up to 6.
                if (rrpv[i] < MAX_FRIENDLY_RRPV) {
                    ++rrpv[i];
                }
            }
        }

        // The newly inserted/accessed friendly line is always
        // given the highest retention priority.
        rrpv[way] = 0;
        break;
    }
}

std::size_t find_victim(std::vector<int>& rrpv)
{
    if (rrpv.empty()) {
        throw std::invalid_argument(
            "Cannot find a victim in an empty RRPV vector");
    }

    while (true) {

        // --------------------------------------------------------
        // First preference:
        //
        // Any cache-averse line has RRPV = 7 and is immediately
        // eligible for eviction.
        // --------------------------------------------------------
        for (std::size_t way = 0; way < rrpv.size(); ++way) {
            if (rrpv[way] == MAX_RRPV) {
                return way;
            }
        }

        // --------------------------------------------------------
        // No line currently has RRPV = 7.
        //
        // Age all lines by one, as in RRIP.
        //
        // Since there is no RRPV=7 currently, incrementing the
        // maximum RRPV will eventually create an eviction
        // candidate.
        // --------------------------------------------------------
        for (int& value : rrpv) {
            if (value < MAX_RRPV) {
                ++value;
            }
        }
    }
}