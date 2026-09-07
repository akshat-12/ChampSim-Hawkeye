#ifndef OPTGEN_H
#define OPTGEN_H

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

class OPTgen {
public:
    // num_sets: number of cache sets tracked independently
    // associativity: W, the cache associativity (occupancy vector cap)
    // history_multiplier: length of tracked history, in units of the set's
    // capacity (paper uses 8x; see Figure 2).
    // Default 8.
    OPTgen(std::size_t num_sets, std::size_t associativity, 
           std::size_t history_multiplier = 8);

    // Processes one access to `address`, mapped to set `set_idx`, per
    // Section 3.1.
    bool access(std::size_t set_idx, uint64_t address);


    // Return the next position in the circular history.
    std::size_t next(std::size_t pos);

private:
    std::size_t num_sets_;
    std::size_t associativity_;
    std::size_t history_length_;

    struct SetState {
        // Occupancy vector used by OPTgen.
        std::vector<std::uint8_t> occupancy;

        // Address stored at each history position.
        std::vector<uint64_t> history;

        // Whether each history position is valid.
        std::vector<bool> valid;

        // Most recent history position for each address.
        std::unordered_map<uint64_t, std::size_t> last_pos;

        // Position where the next access will be inserted.
        std::size_t next_pos = 0;
    };

    // Each cache set has an independent OPTgen state.
    std::vector<SetState> sets_;

};

#endif // OPTGEN_H
