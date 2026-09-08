#include "rrip.h"

#include <algorithm>
#include <stdexcept>

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
    int max_rrpv = INT_MIN;
    int max_rrpv_index = -1;
    for (std::size_t way = 0; way < rrpv.size(); way++) {
        if (rrpv[way] == MAX_RRPV) {
            return way;
        }
        if (rrpv[way] > max_rrpv) {
            max_rrpv = rrpv[way];
            max_rrpv_index = way;
        }
    }
    return static_cast<std::size_t>(max_rrpv_index);
}