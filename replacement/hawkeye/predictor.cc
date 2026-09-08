#include "predictor.h"

#include <stdexcept>

HawkeyePredictor::HawkeyePredictor(std::size_t num_entries,
                                   int counter_bits)
    : num_entries_(num_entries),
      counter_bits_(counter_bits),
      max_counter_(0),
      counters_(num_entries, 0) {

    max_counter_ = (1 << counter_bits_) - 1;
}

void HawkeyePredictor::train(uint64_t pc, bool opt_hit) {
    const std::size_t idx = index(pc);

    if (opt_hit) {
        if (counters_[idx] < max_counter_) {
            counters_[idx]++;
        }
    } else {
        if (counters_[idx] > 0) {
            counters_[idx]--;
        }
    }
}

bool HawkeyePredictor::predict(uint64_t pc) const {
    const std::size_t idx = index(pc);

    // The most valuable bit determines the prediction.
    const int high_bit = 1 << (counter_bits_ - 1);

    return (counters_[idx] & high_bit) != 0;
}

int HawkeyePredictor::get_counter(uint64_t pc) const {
    return counters_[index(pc)];
}

std::size_t HawkeyePredictor::index(uint64_t pc) const {
    constexpr uint64_t pc_mask = (1ULL << 13) - 1;
    return static_cast<std::size_t>((pc & pc_mask) % num_entries_);
}