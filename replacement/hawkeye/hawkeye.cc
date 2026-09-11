// hawkeye.cc

#include "hawkeye.h"

#include <cassert>
#include <cstddef>
#include <stdexcept>

#include "optgen.cc"
#include "predictor.cc"
#include "rrip.cc"

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
      rrpv(static_cast<std::size_t>(sets * ways), MAXRRIP),
      classification(static_cast<std::size_t>(sets * ways),
                     Classification::CACHE_FRIENDLY) {}

long hawkeye::find_victim(
    uint32_t triggering_cpu,
    uint64_t instr_id,
    long set,
    const champsim::cache_block* current_set,
    champsim::address ip,
    champsim::address full_addr,
    access_type type) {

    return static_cast<long>(::find_victim(rrpv[set]));
}

void hawkeye::replacement_cache_fill(
    uint32_t triggering_cpu,
    long set,
    long way,
    champsim::address full_addr,
    champsim::address ip,
    champsim::address victim_addr,
    access_type type) {
    const std::size_t index =
        static_cast<std::size_t>(set * NUM_WAY + way);

    // Query the PC-indexed Hawkeye predictor.
    //
    // predictor = true  -> cache-friendly
    // predictor = false -> cache-averse
    const bool cache_friendly = predictor.predict(ip.to<uint64_t>());

    if (cache_friendly) {
        classification[index] =
            Classification::CACHE_FRIENDLY;
    } else {
        classification[index] =
            Classification::CACHE_AVERSE;
    }

    // A newly inserted line is a miss, so apply the Hawkeye
    // insertion rule.
    update_rrpv(
        rrpv[set],
        static_cast<std::size_t>(way),
        classification[index],
        /*is_hit=*/false);
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
    // ------------------------------------------------------------
    // 1. Run OPTgen for this access.
    //
    // OPTgen works independently for every cache set.
    // ------------------------------------------------------------

    const bool opt_hit =
        optgen.access(
            static_cast<std::size_t>(set),
            full_addr.to<uint64_t>());

    // ------------------------------------------------------------
    // 2. Train the PC-indexed Hawkeye predictor.
    //
    // OPT hit  -> increment counter
    // OPT miss -> decrement counter
    // ------------------------------------------------------------

    predictor.train(
        ip.to<uint64_t>(),
        opt_hit);


    // ------------------------------------------------------------
    // 3. Update RRPV.
    //
    // On a hit, use the classification that was assigned to the
    // line when it was inserted.
    //
    // On a miss, RRPV will be initialized during cache fill,
    // because that is where the new line's predictor classification
    // is known.
    // ------------------------------------------------------------

    if (hit) {
        const std::size_t index =
            static_cast<std::size_t>(set * NUM_WAY + way);

        update_rrpv(
            rrpv_for_set(set),
            static_cast<std::size_t>(way),
            classification[index],
            /*is_hit=*/true);
    }
}
