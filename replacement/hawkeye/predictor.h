#ifndef HAWKEYE_PREDICTOR_H
#define HAWKEYE_PREDICTOR_H

#include <cstddef>
#include <cstdint>
#include <vector>

class HawkeyePredictor {
public:
    // num_entries:
    //   Size of the PC-indexed predictor table.
    //   The Hawkeye paper uses 8K entries.
    //
    // counter_bits:
    //   Width of each saturating counter.
    //   The paper uses 3-bit counters.
    //
    // The high-order bit of the counter determines the
    // classification:
    //
    //   1 -> cache-friendly
    //   0 -> cache-averse
    HawkeyePredictor(std::size_t num_entries = 8192,
                     int counter_bits = 3);

    // Train the counter indexed by a hash of `pc`.
    //
    // OPT hit  -> increment counter
    // OPT miss -> decrement counter
    void train(uint64_t pc, bool opt_hit);

    // Returns the predicted classification for `pc`.
    //
    // true  -> cache-friendly
    // false -> cache-averse
    bool predict(uint64_t pc) const;

    // Debug-only accessor.
    //
    // Returns the raw counter value in:
    //
    // [0, 2^counter_bits - 1]
    int get_counter(uint64_t pc) const;

private:
    std::size_t num_entries_;
    int counter_bits_;
    int max_counter_;

    // PC-indexed table of saturating counters.
    std::vector<int> counters_;

    // Hash PC to a predictor-table index.
    std::size_t index(uint64_t pc) const;
};

#endif // HAWKEYE_PREDICTOR_H