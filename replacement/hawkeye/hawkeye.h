// hawkeye.h
#ifndef HAWKEYE_H
#define HAWKEYE_H
#include <vector>
#include "cache.h"
#include "modules.h"
#include "optgen.h"
#include "predictor.h"
#include "rrip.h"

struct hawkeye : public champsim::modules::replacement {
    long NUM_SET;
    long NUM_WAY;

    // OPTgen state: one independent OPTgen history per cache set
    OPTgen optgen;

    // PC-indexed saturating-counter predictor
    HawkeyePredictor predictor;

    // RRPV state: rrpv[set][way]
    std::vector<std::vector<int>> rrpv;

    std::vector<std::vector<uint64_t>> cache_line_to_pc_mapping;

public:
    explicit hawkeye(CACHE* cache);
    hawkeye(CACHE* cache, long sets, long ways);
    // TODO: instantiate different modules 

    // TODO: Add any new data structures or functions to connect each of
    // the modules

    // TODO: Complete the definitions for the following functions that are
    // required across all replacement policies. You can use the other replacement
    // policies as a reference. Each should be implemented primarily by calling
    // optgen.access(...), predictor.train(...)/predict(...), and
    // update_rrpv(...)/find_victim(...) from rrip.h, do not re-implement
    // OPTgen/predictor/RRIP logic here.

    // find_victim (args);
    // replacement_cache_fill (args);
    // update_replacement_state (args);
    // void initialize_replacement();
    long find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set, const champsim::cache_block* current_set, champsim::address ip,
                    champsim::address full_addr, access_type type);
    void replacement_cache_fill(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip, champsim::address victim_addr,
                                access_type type);
    void update_replacement_state(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip, champsim::address victim_addr,
                                access_type type, uint8_t hit);
};
#endif
