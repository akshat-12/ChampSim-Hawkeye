// hawkeye.cc

#include "hawkeye.h"

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <iostream>

#include "cache.h"

#define MAXRRIP 7

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
      cache_line_to_pc_mapping(static_cast<std::size_t>(sets))  {
      }

long hawkeye::find_victim(
    uint32_t triggering_cpu,
    uint64_t instr_id,
    long set,
    const champsim::cache_block* current_set,
    champsim::address ip,
    champsim::address full_addr,
    access_type type) {

    auto victim = static_cast<long>(::find_victim(rrpv[set]));

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

    // predictor = true  -> cache-friendly
    // predictor = false -> cache-averse
    Classification cls;

    if (predictor.predict(ip.to<uint64_t>())) {
        cls = Classification::CACHE_FRIENDLY;
    } else {
        cls = Classification::CACHE_AVERSE;
    }


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


    const bool opt_hit =
        optgen.access(
            static_cast<std::size_t>(set),
            block_addr);

    if (optgen.last_access_was_reuse(static_cast<std::size_t>(set))) {
        if (cache_line_to_pc_mapping[set].find(block_addr) != cache_line_to_pc_mapping[set].end()) {
            uint64_t victim_pc = cache_line_to_pc_mapping[set][block_addr];
            predictor.train(
                victim_pc,
                opt_hit);
        }
    }
    cache_line_to_pc_mapping[set][block_addr] = ip.to<uint64_t>();

    if (hit) {
        update_rrpv(
            rrpv[set],
            static_cast<std::size_t>(way),
            predictor.predict(ip.to<uint64_t>()) ? Classification::CACHE_FRIENDLY
                                        : Classification::CACHE_AVERSE,
            true /* cache_hit */);
    }
}
