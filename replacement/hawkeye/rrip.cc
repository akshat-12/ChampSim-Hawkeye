#include "rrip.h"

#include <algorithm>
#include <stdexcept>
#include <limits.h>

namespace {

constexpr int MAX_RRPV = 7;

constexpr int MAX_FRIENDLY_RRPV = 6;

} // namespace

void update_rrpv(std::vector<int>& rrpv,
                 std::size_t way,
                 Classification cls,
                 bool is_hit) {

    switch (cls) {
    case Classification::CACHE_AVERSE:
        rrpv[way] = MAX_RRPV;
        break;

    case Classification::CACHE_FRIENDLY:
        if (!is_hit) {
            for (std::size_t i = 0; i < rrpv.size(); ++i) {
                if (rrpv[i] < MAX_FRIENDLY_RRPV) {
                    rrpv[i]++;
                }
            }
        }
        rrpv[way] = 0;
        break;
    }
}

std::size_t find_victim(std::vector<int>& rrpv)
{
    while (true) {
        for (std::size_t way = 0; way < rrpv.size(); ++way) {
            if (rrpv[way] == MAX_RRPV) {
                return way;
            }
        }

        for (int& value : rrpv) {
            ++value;
        }
    }
}