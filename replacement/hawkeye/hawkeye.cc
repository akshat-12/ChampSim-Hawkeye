// hawkeye.cc

#include "hawkeye.h"

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <iostream>

#include "cache.h"

#define MAXRRIP 7

// ================================================================
// Constructor
// ================================================================

hawkeye::hawkeye(CACHE* cache): hawkeye(cache, cache->NUM_SET, cache->NUM_WAY){}

hawkeye::hawkeye(CACHE* cache, long sets, long ways)
    : replacement(cache),
      NUM_SET(sets),
      NUM_WAY(ways),
      optgen(static_cast<std::size_t>(sets),
             static_cast<std::size_t>(ways)),
      predictor(),
     rrpv(static_cast<std::size_t>(sets),
         std::vector<int>(static_cast<std::size_t>(ways), MAXRRIP)),
      cache_line_to_pc_mapping(static_cast<std::size_t>(sets),
                              std::vector<uint64_t>(static_cast<std::size_t>(ways), 0))  {}

long hawkeye::find_victim(
    uint32_t triggering_cpu,
    uint64_t instr_id,
    long set,
    const champsim::cache_block* current_set,
    champsim::address ip,
    champsim::address full_addr,
    access_type type) {
    // std::cout << "Finding victim for PC: " << std::hex << ip.to<uint64_t>() << std::endl;

    auto victim = static_cast<long>(::find_victim(rrpv[set]));

    auto victim_pc = cache_line_to_pc_mapping[set][victim];

    // Detrain the predictor with the victim's PC, since it was not reused.
    predictor.train(
        victim_pc,
        false);

    return victim;
}

void hawkeye::replacement_cache_fill(
    uint32_t triggering_cpu,
    long set,
    long way,
    champsim::address full_addr,
    champsim::address ip,
    champsim::address victim_addr,
    access_type type) {
    // std::cout << "Replacement PC: " << std::hex << ip.to<uint64_t>() << std::endl;
    // Query the PC-indexed Hawkeye predictor.
    //
    // predictor = true  -> cache-friendly
    // predictor = false -> cache-averse
    if (type == access_type::WRITE) {
        rrpv[set][way] = MAXRRIP;
        cache_line_to_pc_mapping[set][way] = ip.to<uint64_t>();
        return;
    }
    Classification cls;

    if (predictor.predict(ip.to<uint64_t>())) {
        cls = Classification::CACHE_FRIENDLY;
    } else {
        cls = Classification::CACHE_AVERSE;
    }

    cache_line_to_pc_mapping[set][way] = ip.to<uint64_t>();

    // A newly inserted line is a miss, so apply the Hawkeye
    // insertion rule.
    update_rrpv(
        rrpv[set],
        static_cast<std::size_t>(way),
        cls,
        false /*is_hit=*/);
}

void hawkeye::update_replacement_state(
    uint32_t triggering_cpu,
    long set,
    long way,
    champsim::address full_addr,
    champsim::address ip,
    champsim::address victim_addr,
    access_type type,
    uint8_t hit) {

    uint64_t block_addr = (full_addr.to<uint64_t>() >> 6) << 6;
    // if (!hit) {
    //     std::cout << "Miss for PC: " << std::hex << ip.to<uint64_t>() << std::endl;
    // }


    const bool opt_hit =
        optgen.access(
            static_cast<std::size_t>(set),
            block_addr);

    if (optgen.last_access_was_reuse(static_cast<std::size_t>(set))) {
        predictor.train(
            ip.to<uint64_t>(),
            opt_hit);
    }

    if (hit) {
        update_rrpv(
            rrpv[set],
            static_cast<std::size_t>(way),
            predictor.predict(ip.to<uint64_t>()) ? Classification::CACHE_FRIENDLY
                                        : Classification::CACHE_AVERSE,
            true /* cache_hit */);
    }
}
