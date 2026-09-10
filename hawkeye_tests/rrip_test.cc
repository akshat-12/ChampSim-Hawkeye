// rrip_test.cc
#include "../replacement/hawkeye/rrip.cc"
#include <iostream>
#include <vector>
#include <cstddef>

int main()
{
    // ============================================================
    // Case 1: Cache-averse insertion
    //
    // A cache-averse line should be inserted with RRPV = 7.
    // ============================================================
    std::vector<int> rrpv1 = {0, 0, 0, 0};

    update_rrpv(rrpv1, 0,
                Classification::CACHE_AVERSE,
                /*is_hit=*/false);

    std::cout << "Case 1 - cache-averse insertion:\n";
    for (int v : rrpv1)
        std::cout << v << " ";
    std::cout << "\n";

    // Expected:
    // 7 0 0 0


    // ============================================================
    // Case 2: Cache-friendly insertion
    //
    // On a cache-friendly miss:
    //   1. Age all lines with RRPV < 6
    //   2. Set inserted line's RRPV = 0
    // ============================================================
    std::vector<int> rrpv2 = {0, 1, 2, 5};

    update_rrpv(rrpv2, 1,
                Classification::CACHE_FRIENDLY,
                /*is_hit=*/false);

    std::cout << "Case 2 - cache-friendly insertion:\n";
    for (int v : rrpv2)
        std::cout << v << " ";
    std::cout << "\n";

    // Before:
    // 0 1 2 5
    //
    // Age:
    // 1 2 3 6
    //
    // Set way 1 to 0:
    // 1 0 3 6
    //
    // Expected:
    // 1 0 3 6


    // ============================================================
    // Case 3: Cache-friendly hit
    //
    // A cache-friendly hit should set that way's RRPV to 0.
    // No other line should be aged.
    // ============================================================
    std::vector<int> rrpv3 = {2, 4, 5, 6};

    update_rrpv(rrpv3, 2,
                Classification::CACHE_FRIENDLY,
                /*is_hit=*/true);

    std::cout << "Case 3 - cache-friendly hit:\n";
    for (int v : rrpv3)
        std::cout << v << " ";
    std::cout << "\n";

    // Expected:
    // 2 4 0 6


    // ============================================================
    // Case 4: Cache-averse hit
    //
    // Cache-averse hit -> RRPV = 7.
    // ============================================================
    std::vector<int> rrpv4 = {1, 2, 3, 4};

    update_rrpv(rrpv4, 2,
                Classification::CACHE_AVERSE,
                /*is_hit=*/true);

    std::cout << "Case 4 - cache-averse hit:\n";
    for (int v : rrpv4)
        std::cout << v << " ";
    std::cout << "\n";

    // Expected:
    // 1 2 7 4


    // ============================================================
    // Case 5: Cache-friendly miss with RRPV = 6
    //
    // RRPV = 6 should NOT be incremented because the Hawkeye
    // insertion rule only ages values < 6.
    // ============================================================
    std::vector<int> rrpv5 = {0, 5, 6, 7};

    update_rrpv(rrpv5, 1,
                Classification::CACHE_FRIENDLY,
                /*is_hit=*/false);

    std::cout << "Case 5 - aging boundary:\n";
    for (int v : rrpv5)
        std::cout << v << " ";
    std::cout << "\n";

    // Age:
    // 0 -> 1
    // 5 -> 6
    // 6 -> 6
    // 7 -> 7
    //
    // Then way 1 -> 0
    //
    // Expected:
    // 1 0 6 7


    // ============================================================
    // Case 6: Victim selection with RRPV = 7
    //
    // If any line has RRPV = 7, choose it immediately.
    // ============================================================
    std::vector<int> rrpv6 = {2, 7, 4, 7};

    std::size_t victim6 = find_victim(rrpv6);

    std::cout << "Case 6 - RRPV 7 victim:\n";
    for (int v : rrpv6)
        std::cout << v << " ";
    std::cout << "\n";
    std::cout << "victim: " << victim6 << "\n";

    // Expected:
    // victim = 1
    //
    // First RRPV=7 is selected.
    // No aging should occur.
    //
    // Expected RRPV:
    // 2 7 4 7


    // ============================================================
    // Case 7: Victim selection without RRPV = 7
    //
    // If no line has RRPV = 7, choose the line with the highest
    // RRPV. DO NOT age the lines.
    // ============================================================
    std::vector<int> rrpv7 = {1, 3, 5, 2};

    std::size_t victim7 = find_victim(rrpv7);

    std::cout << "Case 7 - highest RRPV victim:\n";
    for (int v : rrpv7)
        std::cout << v << " ";
    std::cout << "\n";
    std::cout << "victim: " << victim7 << "\n";

    // Expected:
    // victim = 2
    //
    // RRPV should remain:
    // 1 3 5 2


    // ============================================================
    // Case 8: Victim selection with a tie
    //
    // Multiple lines can have the same maximum RRPV.
    // Your implementation should deterministically select the
    // first one.
    // ============================================================
    std::vector<int> rrpv8 = {3, 5, 2, 5};

    std::size_t victim8 = find_victim(rrpv8);

    std::cout << "Case 8 - maximum RRPV tie:\n";
    for (int v : rrpv8)
        std::cout << v << " ";
    std::cout << "\n";
    std::cout << "victim: " << victim8 << "\n";

    // Expected:
    // victim = 1
    //
    // Both way 1 and way 3 have RRPV = 5.
    // First maximum is selected.


    // ============================================================
    // Case 9: All lines are cache-averse
    //
    // Every line has RRPV = 7. The first way should be selected.
    // ============================================================
    std::vector<int> rrpv9 = {7, 7, 7, 7};

    std::size_t victim9 = find_victim(rrpv9);

    std::cout << "Case 9 - all RRPV 7:\n";
    for (int v : rrpv9)
        std::cout << v << " ";
    std::cout << "\n";
    std::cout << "victim: " << victim9 << "\n";

    // Expected:
    // victim = 0


    // ============================================================
    // Case 10: Complete insertion + victim sequence
    //
    // Start with an empty-ish set represented by RRPV = 7.
    // Insert a cache-friendly line into way 0.
    // ============================================================
    std::vector<int> rrpv10 = {7, 7, 7, 7};

    update_rrpv(rrpv10, 0,
                Classification::CACHE_FRIENDLY,
                /*is_hit=*/false);

    std::cout << "Case 10 - insertion sequence:\n";
    for (int v : rrpv10)
        std::cout << v << " ";
    std::cout << "\n";

    std::size_t victim10 = find_victim(rrpv10);

    std::cout << "victim: " << victim10 << "\n";

    // Expected after friendly insertion:
    // 0 7 7 7
    //
    // Expected victim:
    // 1
}
