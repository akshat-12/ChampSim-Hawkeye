#include "predictor.h"

#include <stdexcept>

HawkeyePredictor::HawkeyePredictor(std::size_t num_entries,
                                   int counter_bits)
    : num_entries_(num_entries),
      counter_bits_(counter_bits),
      max_counter_(0),
      counters_(num_entries, 0)
{
    if (num_entries_ == 0) {
        throw std::invalid_argument(
            "num_entries must be greater than zero");
    }

    if (counter_bits_ <= 0 || counter_bits_ > 30) {
        throw std::invalid_argument(
            "counter_bits must be in the range [1, 30]");
    }

    max_counter_ = (1 << counter_bits_) - 1;
}

void HawkeyePredictor::train(uint64_t pc, bool opt_hit)
{
    const std::size_t idx = index(pc);

    if (opt_hit) {
        // Train toward cache-friendly.
        //
        // Saturating increment:
        //
        // 0 -> 1 -> 2 -> ... -> max
        if (counters_[idx] < max_counter_) {
            ++counters_[idx];
        }
    } else {
        // Train toward cache-averse.
        //
        // Saturating decrement:
        //
        // max -> ... -> 2 -> 1 -> 0
        if (counters_[idx] > 0) {
            --counters_[idx];
        }
    }
}

bool HawkeyePredictor::predict(uint64_t pc) const
{
    const std::size_t idx = index(pc);

    // The high-order bit determines the prediction.
    //
    // For the default 3-bit counter:
    //
    //   000 -> averse
    //   001 -> averse
    //   010 -> averse
    //   011 -> averse
    //   100 -> friendly
    //   101 -> friendly
    //   110 -> friendly
    //   111 -> friendly
    //
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