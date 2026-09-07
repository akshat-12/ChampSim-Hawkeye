#ifndef HAWKEYE_RRIP_H
#define HAWKEYE_RRIP_H

#include <cstddef>
#include <vector>

enum class Classification {
    CACHE_FRIENDLY,
    CACHE_AVERSE
};

// Applies Hawkeye Table 1's update rule to a set's RRPV vector.
//
// `way`:
//   Way being accessed/inserted.
//
// `cls`:
//   Hawkeye prediction for the accessed line.
//
// `is_hit`:
//   true  -> cache hit
//   false -> cache miss / insertion
//
// RRPV values are 3-bit values in [0, 7].
void update_rrpv(std::vector<int>& rrpv,
                 std::size_t way,
                 Classification cls,
                 bool is_hit);

// Selects a victim according to Hawkeye Section 3.4.
//
// First prefers a line with RRPV = 7.
// If none exists, ages the set until at least one line
// reaches RRPV = 7, then selects a victim.
//
// Returns the victim way.
std::size_t find_victim(std::vector<int>& rrpv);

#endif // HAWKEYE_RRIP_H