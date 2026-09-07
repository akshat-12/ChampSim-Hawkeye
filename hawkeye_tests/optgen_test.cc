#include "../replacement/hawkeye/optgen.cc"
#include <iostream>
#include <vector>
#include <utility>
#include <cstdint>

int main() {
    OPTgen opt(/*num_sets=*/1, /*associativity=*/2);

    // ------------------------------------------------------------
    // Test cases
    //
    // W = 2, so OPT can keep at most 2 live blocks per set.
    // ------------------------------------------------------------
    std::vector<std::pair<std::size_t, uint64_t>> accesses = {

        // First occurrences -> MISS
        {0, 0xA},
        {0, 0xB},

        // Reuse of A and B -> should be HIT
        {0, 0xA},
        {0, 0xB},

        // New block -> MISS
        {0, 0xC},

        // A is reused after C.
        {0, 0xA},

        // B is reused.
        {0, 0xB},

        // New block D.
        {0, 0xD},

        // Reuse C.
        {0, 0xC},

        // Reuse D.
        {0, 0xD},

        // Reuse A.
        {0, 0xA},

        // Reuse B.
        {0, 0xB},
    };

    int hits = 0;

    for (auto& access : accesses) {
        auto& set_idx = access.first;
        auto& addr = access.second;
        bool hit = opt.access(set_idx, addr);

        std::cout << std::hex << addr
                  << std::dec << ": "
                  << (hit ? "HIT" : "MISS")
                  << "\n";

        if (hit)
            hits++;
    }

    std::cout << "TOTAL HITS: " << hits << "\n";
}
