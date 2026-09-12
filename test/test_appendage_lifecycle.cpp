#include <gtest/gtest.h>
#include <aitvaras/marshaler.hpp>
#include "protocols.hpp"
#include <cstring>
#include <cstdint>

using namespace aitvaras;

// ============================================================
// Appendage Lifecycle Tests
//   Each test exercises the full add → verify → add more →
//   verify → remove → verify → re-add → verify pattern.
// ============================================================

// Helper: verify total_size_ exactly matches fixed_size + sum of (2 + len) for each present appendage.
// We also round-trip through a reader to confirm the buffer is coherent at every step.

TEST(AppendageLifecycle, OuchEnterOrder_AddVerifyRemoveReadd) {
    char buffer[1024] = {0};

    // ---- Build the fixed fields ----
    Marshaler<OuchEnterOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    (void)(o.type = 'O');
    (void)(o.userRefNum = 42);
    (void)(o.side = 'B');
    (void)(o.quantity = 100);
    (void)(o.symbol = "NVDA");
    (void)(o.price = 500.0);
    (void)(o.timeInForce = '0');
    (void)(o.display = 'Y');
    (void)(o.capacity = 'A');
    (void)(o.interMarketSweepEligibility = 'N');
    (void)(o.crossType = 'N');
    (void)(o.clOrdID = "LIFE1");

    const size_t fixed_size = 47;  // 1+4+1+4+8+8+1+1+1+1+1+14+2
    EXPECT_EQ(o.state_.total_size_, fixed_size);

    // =========================================================
    // STEP 1: Add Firm appendage, verify
    // =========================================================
    const uint8_t firm1[] = {'F', 'I', 'R', 'M'};
    EXPECT_TRUE((o.appendageFirm = std::span<const uint8_t>(firm1, 4)).has_value());

    // Fixed + 1 frame (2 + 4)
    EXPECT_EQ(o.state_.total_size_, fixed_size + 6u);

    {
        size_t sz = o.state_.total_size_;
        Marshaler<OuchEnterOrder, true> r{{std::span<const char>(buffer, sz)}};
        auto firm = r.appendageFirm.get();
        ASSERT_TRUE(firm.has_value());
        EXPECT_EQ((*firm)[0], 'F');
        EXPECT_EQ((*firm)[3], 'M');
        // Others absent
        EXPECT_FALSE(r.appendageMinQty.get().has_value());
        EXPECT_FALSE(r.appendageRoute.get().has_value());
        EXPECT_FALSE(r.appendagePegOffset.get().has_value());
    }

    // =========================================================
    // STEP 2: Add MinQty and Route appendages, verify all three
    // =========================================================
    const uint8_t minqty1[] = {0x00, 0x00, 0x00, 0x05};  // uint32 = 5
    const uint8_t route1[]  = {'I', 'N', 'T', 'L'};

    EXPECT_TRUE((o.appendageMinQty = std::span<const uint8_t>(minqty1, 4)).has_value());
    EXPECT_TRUE((o.appendageRoute  = std::span<const uint8_t>(route1,  4)).has_value());

    // Fixed + 3 frames each (2 + 4)
    EXPECT_EQ(o.state_.total_size_, fixed_size + 3 * 6u);

    {
        size_t sz = o.state_.total_size_;
        Marshaler<OuchEnterOrder, true> r{{std::span<const char>(buffer, sz)}};

        auto firm = r.appendageFirm.get();
        ASSERT_TRUE(firm.has_value());
        EXPECT_EQ((*firm)[0], 'F');

        auto minqty = r.appendageMinQty.get();
        ASSERT_TRUE(minqty.has_value());
        EXPECT_EQ((*minqty)[3], 0x05u);

        auto route = r.appendageRoute.get();
        ASSERT_TRUE(route.has_value());
        EXPECT_EQ((*route)[0], 'I');
        EXPECT_EQ((*route)[3], 'L');

        // Others still absent
        EXPECT_FALSE(r.appendagePegOffset.get().has_value());
        EXPECT_FALSE(r.appendageHandleInst.get().has_value());
    }

    // =========================================================
    // STEP 3: Remove Firm (the first appendage added), verify
    //         MinQty and Route survive intact; Firm is gone
    // =========================================================
    EXPECT_TRUE(o.appendageFirm.remove().has_value());

    // Fixed + 2 frames
    EXPECT_EQ(o.state_.total_size_, fixed_size + 2 * 6u);

    {
        size_t sz = o.state_.total_size_;
        Marshaler<OuchEnterOrder, true> r{{std::span<const char>(buffer, sz)}};

        // Firm is gone
        EXPECT_FALSE(r.appendageFirm.get().has_value());

        // MinQty and Route are unaffected
        auto minqty = r.appendageMinQty.get();
        ASSERT_TRUE(minqty.has_value());
        EXPECT_EQ((*minqty)[3], 0x05u);

        auto route = r.appendageRoute.get();
        ASSERT_TRUE(route.has_value());
        EXPECT_EQ((*route)[0], 'I');
        EXPECT_EQ((*route)[3], 'L');
    }

    // Removing again is a no-op (should succeed)
    EXPECT_TRUE(o.appendageFirm.remove().has_value());
    EXPECT_EQ(o.state_.total_size_, fixed_size + 2 * 6u);

    // =========================================================
    // STEP 4: Re-add Firm with different content, verify all
    //         three appendages again with correct values
    // =========================================================
    const uint8_t firm2[] = {'A', 'C', 'M', 'E'};
    EXPECT_TRUE((o.appendageFirm = std::span<const uint8_t>(firm2, 4)).has_value());

    // Back to 3 frames
    EXPECT_EQ(o.state_.total_size_, fixed_size + 3 * 6u);

    {
        size_t sz = o.state_.total_size_;
        Marshaler<OuchEnterOrder, true> r{{std::span<const char>(buffer, sz)}};

        // Firm is back with new content
        auto firm = r.appendageFirm.get();
        ASSERT_TRUE(firm.has_value());
        EXPECT_EQ((*firm)[0], 'A');
        EXPECT_EQ((*firm)[1], 'C');
        EXPECT_EQ((*firm)[2], 'M');
        EXPECT_EQ((*firm)[3], 'E');

        // MinQty untouched
        auto minqty = r.appendageMinQty.get();
        ASSERT_TRUE(minqty.has_value());
        EXPECT_EQ((*minqty)[3], 0x05u);

        // Route untouched
        auto route = r.appendageRoute.get();
        ASSERT_TRUE(route.has_value());
        EXPECT_EQ((*route)[0], 'I');
        EXPECT_EQ((*route)[3], 'L');
    }
}

TEST(AppendageLifecycle, OuchEnterOrder_RemoveMiddleAppendage) {
    // Verifies that removing the middle of three appendages correctly
    // slides the third one forward and fixes its offset.
    char buffer[1024] = {0};

    Marshaler<OuchEnterOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    (void)(o.type = 'O');
    (void)(o.userRefNum = 9);
    (void)(o.side = 'S');
    (void)(o.quantity = 50);
    (void)(o.symbol = "META");
    (void)(o.price = 300.0);
    (void)(o.timeInForce = '0');
    (void)(o.display = 'Y');
    (void)(o.capacity = 'A');
    (void)(o.interMarketSweepEligibility = 'N');
    (void)(o.crossType = 'N');
    (void)(o.clOrdID = "MID");

    const uint8_t firm_data[]   = {'X', 'X', 'X', 'X'};
    const uint8_t minqty_data[] = {0x00, 0x00, 0x00, 0x0A};
    const uint8_t route_data[]  = {'N', 'Y', 'S', 'E'};

    EXPECT_TRUE((o.appendageFirm   = std::span<const uint8_t>(firm_data,   4)).has_value());
    EXPECT_TRUE((o.appendageMinQty = std::span<const uint8_t>(minqty_data, 4)).has_value());
    EXPECT_TRUE((o.appendageRoute  = std::span<const uint8_t>(route_data,  4)).has_value());

    const size_t fixed_size = 47;
    EXPECT_EQ(o.state_.total_size_, fixed_size + 18u);  // 3 × 6

    // Remove the middle (MinQty)
    EXPECT_TRUE(o.appendageMinQty.remove().has_value());
    EXPECT_EQ(o.state_.total_size_, fixed_size + 12u);  // 2 × 6

    {
        size_t sz = o.state_.total_size_;
        Marshaler<OuchEnterOrder, true> r{{std::span<const char>(buffer, sz)}};

        auto firm = r.appendageFirm.get();
        ASSERT_TRUE(firm.has_value());
        EXPECT_EQ((*firm)[0], 'X');

        EXPECT_FALSE(r.appendageMinQty.get().has_value());

        auto route = r.appendageRoute.get();
        ASSERT_TRUE(route.has_value());
        EXPECT_EQ((*route)[0], 'N');
        EXPECT_EQ((*route)[2], 'S');
        EXPECT_EQ((*route)[3], 'E');
    }
}

TEST(AppendageLifecycle, OuchEnterOrder_RemoveLastAppendage) {
    // Verifies that removing the last appendage does not disturb earlier ones.
    char buffer[1024] = {0};

    Marshaler<OuchEnterOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    (void)(o.type = 'O');
    (void)(o.userRefNum = 77);
    (void)(o.side = 'B');
    (void)(o.quantity = 200);
    (void)(o.symbol = "AAPL");
    (void)(o.price = 175.0);
    (void)(o.timeInForce = '0');
    (void)(o.display = 'Y');
    (void)(o.capacity = 'A');
    (void)(o.interMarketSweepEligibility = 'N');
    (void)(o.crossType = 'N');
    (void)(o.clOrdID = "LAST");

    const uint8_t firm_data[]  = {'A', 'B', 'C', 'D'};
    const uint8_t route_data[] = {'L', 'A', 'S', 'T'};

    EXPECT_TRUE((o.appendageFirm  = std::span<const uint8_t>(firm_data,  4)).has_value());
    EXPECT_TRUE((o.appendageRoute = std::span<const uint8_t>(route_data, 4)).has_value());

    const size_t fixed_size = 47;
    EXPECT_EQ(o.state_.total_size_, fixed_size + 12u);

    // Remove last (Route)
    EXPECT_TRUE(o.appendageRoute.remove().has_value());
    EXPECT_EQ(o.state_.total_size_, fixed_size + 6u);

    {
        size_t sz = o.state_.total_size_;
        Marshaler<OuchEnterOrder, true> r{{std::span<const char>(buffer, sz)}};

        auto firm = r.appendageFirm.get();
        ASSERT_TRUE(firm.has_value());
        EXPECT_EQ((*firm)[0], 'A');
        EXPECT_EQ((*firm)[3], 'D');

        EXPECT_FALSE(r.appendageRoute.get().has_value());
    }
}

TEST(AppendageLifecycle, OuchEnterOrder_RemoveAll_ThenReadd) {
    // Remove all three appendages one by one, ending with an empty TLV region,
    // then re-add them in reverse order to verify the buffer is fully coherent.
    char buffer[1024] = {0};

    Marshaler<OuchEnterOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    (void)(o.type = 'O');
    (void)(o.userRefNum = 3);
    (void)(o.side = 'B');
    (void)(o.quantity = 10);
    (void)(o.symbol = "IBM");
    (void)(o.price = 140.0);
    (void)(o.timeInForce = '0');
    (void)(o.display = 'Y');
    (void)(o.capacity = 'A');
    (void)(o.interMarketSweepEligibility = 'N');
    (void)(o.crossType = 'N');
    (void)(o.clOrdID = "ALL");

    const uint8_t firm_data[]   = {'F', 'F', 'F', 'F'};
    const uint8_t minqty_data[] = {0x00, 0x00, 0x00, 0x03};
    const uint8_t route_data[]  = {'R', 'R', 'R', 'R'};

    EXPECT_TRUE((o.appendageFirm   = std::span<const uint8_t>(firm_data,   4)).has_value());
    EXPECT_TRUE((o.appendageMinQty = std::span<const uint8_t>(minqty_data, 4)).has_value());
    EXPECT_TRUE((o.appendageRoute  = std::span<const uint8_t>(route_data,  4)).has_value());

    const size_t fixed_size = 47;
    EXPECT_EQ(o.state_.total_size_, fixed_size + 18u);

    // Remove all
    EXPECT_TRUE(o.appendageFirm.remove().has_value());
    EXPECT_EQ(o.state_.total_size_, fixed_size + 12u);

    EXPECT_TRUE(o.appendageMinQty.remove().has_value());
    EXPECT_EQ(o.state_.total_size_, fixed_size + 6u);

    EXPECT_TRUE(o.appendageRoute.remove().has_value());
    EXPECT_EQ(o.state_.total_size_, fixed_size);

    // Verify truly empty TLV region (size == fixed_size, no appendages)
    {
        size_t sz = o.state_.total_size_;
        Marshaler<OuchEnterOrder, true> r{{std::span<const char>(buffer, sz)}};
        EXPECT_FALSE(r.appendageFirm.get().has_value());
        EXPECT_FALSE(r.appendageMinQty.get().has_value());
        EXPECT_FALSE(r.appendageRoute.get().has_value());
    }

    // Re-add in reverse order
    const uint8_t route2[] = {'N', 'E', 'W', 'R'};
    const uint8_t minqty2[] = {0x00, 0x00, 0x00, 0xFF};
    const uint8_t firm2[]  = {'N', 'E', 'W', 'F'};

    EXPECT_TRUE((o.appendageRoute  = std::span<const uint8_t>(route2,  4)).has_value());
    EXPECT_TRUE((o.appendageMinQty = std::span<const uint8_t>(minqty2, 4)).has_value());
    EXPECT_TRUE((o.appendageFirm   = std::span<const uint8_t>(firm2,   4)).has_value());

    EXPECT_EQ(o.state_.total_size_, fixed_size + 18u);

    {
        size_t sz = o.state_.total_size_;
        Marshaler<OuchEnterOrder, true> r{{std::span<const char>(buffer, sz)}};

        auto firm = r.appendageFirm.get();
        ASSERT_TRUE(firm.has_value());
        EXPECT_EQ((*firm)[0], 'N');
        EXPECT_EQ((*firm)[3], 'F');

        auto minqty = r.appendageMinQty.get();
        ASSERT_TRUE(minqty.has_value());
        EXPECT_EQ((*minqty)[3], 0xFFu);

        auto route = r.appendageRoute.get();
        ASSERT_TRUE(route.has_value());
        EXPECT_EQ((*route)[0], 'N');
        EXPECT_EQ((*route)[2], 'W');
    }
}

TEST(AppendageLifecycle, OuchReplaceOrder_VariableSize_RemoveReadd) {
    // Test lifecycle on ReplaceOrder to verify the feature is not EnterOrder-specific.
    // Also use a variable-sized appendage to test size changes across remove/re-add.
    char buffer[1024] = {0};

    Marshaler<OuchReplaceOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    (void)(o.type = 'U');
    (void)(o.origUserRefNum = 1);
    (void)(o.userRefNum = 2);
    (void)(o.quantity = 500);
    (void)(o.price = 99.0);
    (void)(o.timeInForce = '0');
    (void)(o.display = 'Y');
    (void)(o.interMarketSweepEligibility = 'N');
    (void)(o.clOrdID = "LCY");

    // Step 1: Add HandleInst (1 byte) + Firm (4 bytes)
    const uint8_t hi1[] = {0x02};
    const uint8_t f1[]  = {'O', 'R', 'D', 'R'};

    EXPECT_TRUE((o.appendageHandleInst = std::span<const uint8_t>(hi1, 1)).has_value());
    EXPECT_TRUE((o.appendageFirm       = std::span<const uint8_t>(f1,  4)).has_value());

    size_t sz_after_two = o.state_.total_size_;

    // Step 2: Verify both
    {
        Marshaler<OuchReplaceOrder, true> r{{std::span<const char>(buffer, sz_after_two)}};
        auto hi = r.appendageHandleInst.get();
        ASSERT_TRUE(hi.has_value());
        EXPECT_EQ((*hi)[0], 0x02u);

        auto firm = r.appendageFirm.get();
        ASSERT_TRUE(firm.has_value());
        EXPECT_EQ((*firm)[0], 'O');
    }

    // Step 3: Remove HandleInst
    EXPECT_TRUE(o.appendageHandleInst.remove().has_value());
    EXPECT_EQ(o.state_.total_size_, sz_after_two - 3u);  // lost 2+1=3 bytes

    {
        size_t sz = o.state_.total_size_;
        Marshaler<OuchReplaceOrder, true> r{{std::span<const char>(buffer, sz)}};
        EXPECT_FALSE(r.appendageHandleInst.get().has_value());

        auto firm = r.appendageFirm.get();
        ASSERT_TRUE(firm.has_value());
        EXPECT_EQ((*firm)[0], 'O');
    }

    // Step 4: Re-add HandleInst with a different value
    const uint8_t hi2[] = {0x07};
    EXPECT_TRUE((o.appendageHandleInst = std::span<const uint8_t>(hi2, 1)).has_value());
    EXPECT_EQ(o.state_.total_size_, sz_after_two);

    {
        size_t sz = o.state_.total_size_;
        Marshaler<OuchReplaceOrder, true> r{{std::span<const char>(buffer, sz)}};

        auto hi = r.appendageHandleInst.get();
        ASSERT_TRUE(hi.has_value());
        EXPECT_EQ((*hi)[0], 0x07u);  // new value

        auto firm = r.appendageFirm.get();
        ASSERT_TRUE(firm.has_value());
        EXPECT_EQ((*firm)[0], 'O');   // unchanged
    }
}
