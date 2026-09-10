// predictor_test.cc
#include "../replacement/hawkeye/predictor.cc"
#include <iostream>
#include <vector>
#include <utility>
#include <cstdint>

int main() {
    HawkeyePredictor pred;

    // TEST_VECTOR_START
    // train_events: ordered (pc, opt_hit) pairs applied via pred.train(pc,
    // opt_hit)
    std::vector<std::pair<uint64_t, bool>> train_events = {

        // ------------------------------------------------------------
        // PC 0x1000
        // Start at counter = 0.
        // 4 OPT hits should move:
        //   0 -> 1 -> 2 -> 3 -> 4
        // Counter 4 has its high bit set => CACHE-FRIENDLY.
        // ------------------------------------------------------------
        {0x1000, true},
        {0x1000, true},
        {0x1000, true},
        {0x1000, true},

        // ------------------------------------------------------------
        // PC 0x2000
        // Start at counter = 0.
        // OPT misses should saturate at 0.
        // ------------------------------------------------------------
        {0x2000, false},
        {0x2000, false},

        // ------------------------------------------------------------
        // PC 0x3000
        // Test maximum saturation.
        // 7 hits should move counter to 7.
        // Additional hits must leave it at 7.
        // ------------------------------------------------------------
        {0x3000, true},
        {0x3000, true},
        {0x3000, true},
        {0x3000, true},
        {0x3000, true},
        {0x3000, true},
        {0x3000, true},
        {0x3000, true},   // extra hit: must remain 7

        // ------------------------------------------------------------
        // PC 0x4000
        // Reach counter = 4, then decrement.
        //
        // 4 hits:
        //   0 -> 4
        //
        // 1 miss:
        //   4 -> 3
        //
        // This tests the prediction boundary.
        // ------------------------------------------------------------
        {0x4000, true},
        {0x4000, true},
        {0x4000, true},
        {0x4000, true},
        {0x4000, false},

        // ------------------------------------------------------------
        // PC 0x5000
        // Mixed training.
        //
        // 5 hits: 0 -> 5
        // 2 misses: 5 -> 3
        // 1 hit:    3 -> 4
        //
        // Final counter = 4 => cache-friendly.
        // ------------------------------------------------------------
        {0x5000, true},
        {0x5000, true},
        {0x5000, true},
        {0x5000, true},
        {0x5000, true},
        {0x5000, false},
        {0x5000, false},
        {0x5000, true},
    };

    // query_pcs: pcs to print counter/prediction for, after all training
    // is applied
    std::vector<uint64_t> query_pcs = {
        0x1000,
        0x2000,
        0x3000,
        0x4000,
        0x5000,
    };

    // TEST_VECTOR_END

    for (auto& [pc, opt_hit] : train_events) {
        pred.train(pc, opt_hit);
    }

    for (uint64_t pc : query_pcs) {
        std::cout << std::hex << pc << std::dec
                  << ": counter=" << pred.get_counter(pc)
                  << " predict=" << pred.predict(pc) << "\n";
    }
}
