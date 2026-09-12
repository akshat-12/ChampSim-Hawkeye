#include "optgen.h"

OPTgen::OPTgen(std::size_t num_sets,
               std::size_t associativity,
               std::size_t history_multiplier) {

    // Set the member variables based on the constructor parameters.
    num_sets_ = num_sets;
    associativity_ = associativity;
    history_length_ = associativity * history_multiplier;

    sets_.resize(num_sets_);
    last_access_was_reuse_.resize(num_sets_, false);

    // Initialize the vector of SetState objects for each cache set.
    for (auto& set : sets_) {
        set.occupancy.resize(history_length_, 0);
        set.history.resize(history_length_);
        set.valid.resize(history_length_, false);
    }
}

bool OPTgen::access(std::size_t set_idx, uint64_t address) {
    SetState& set = sets_[set_idx];

    // Variables to be used for the current set (for code readability)
    std::vector<uint64_t>& history = set.history;
    std::vector<int>& occupancy = set.occupancy;
    std::vector<bool>& valid = set.valid;
    std::unordered_map<uint64_t, std::size_t>& last_pos = set.last_pos;
    const std::size_t current_pos = set.next_pos;

    // Remove the old address occupying this history position
    if (valid[current_pos]) {
        uint64_t old_address = history[current_pos];

        auto it = last_pos.find(old_address);

        if (it != last_pos.end() &&
            it->second == current_pos) {
            last_pos.erase(it);
        }
    }

    // 0 because cache bypass
    occupancy[current_pos] = 0;

    auto it = last_pos.find(address);

    if (it == set.last_pos.end()) {

        last_access_was_reuse_[set_idx] = false;
        // No previous occurrence in the tracked history.
        history[current_pos] = address;
        valid[current_pos] = true;
        last_pos[address] = current_pos;
        
        // Advance the next position in the circular history.
        set.next_pos = next(set.next_pos);

        return false;
    }

    last_access_was_reuse_[set_idx] = true;

    const std::size_t previous_pos = it->second;
    bool opt_hit = true;

    std::size_t pos = previous_pos;

    while (pos != current_pos) {
        if (occupancy[pos] >= associativity_) {
            opt_hit = false;
            break;
        }

        pos = next(pos);
    }

    if (opt_hit) {
        pos = previous_pos;

        while (pos != current_pos) {
            ++occupancy[pos];
            pos = next(pos);
        }
    }

    // Record the current access in the history.
    history[current_pos] = address;
    valid[current_pos] = true;
    last_pos[address] = current_pos;

    // Advance the next position in the circular history.
    set.next_pos = next(set.next_pos);

    return opt_hit;
}

std::size_t OPTgen::next(std::size_t pos) {
    pos++;
    if (pos == history_length_) {
        pos = 0;
    }
    return pos;
}

bool OPTgen::last_access_was_reuse(std::size_t set_idx) const {
    return last_access_was_reuse_[set_idx];
}