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

void HawkeyePredictor::train(uint64_t pc, bool opt_hit)
{
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

bool HawkeyePredictor::predict(uint64_t pc) const
{
    const std::size_t idx = index(pc);

    // The most valuable bit determines the prediction.
    const int high_bit = 1 << (counter_bits_ - 1);

    return (counters_[idx] & high_bit) != 0;
}

int HawkeyePredictor::get_counter(uint64_t pc) const
{
    return counters_[index(pc)];
}

std::size_t HawkeyePredictor::index(uint64_t pc) const
{
    // 64-bit mixing function followed by table indexing.
    //
    // The paper describes the predictor as an 8K-entry table
    // indexed by a hashed 13-bit PC.
    uint64_t x = pc;

    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;

    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;

    x ^= x >> 33;

    return static_cast<std::size_t>(x % num_entries_);
}