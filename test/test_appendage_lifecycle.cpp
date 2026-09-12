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
    EXPECT_TRUE((o.appendageFirm = "FIRM").has_value());

    // Fixed + 1 frame (2 + 4)
    EXPECT_EQ(o.state_.total_size_, fixed_size + 6u);

    {
        size_t sz = o.state_.total_size_;
        Marshaler<OuchEnterOrder, true> r{{std::span<const char>(buffer, sz)}};
        auto firm = r.appendageFirm.get();
        ASSERT_TRUE(firm.has_value());
        EXPECT_EQ(*firm, "FIRM");
        // Others absent
        EXPECT_FALSE(r.appendageMinQty.get().has_value());
        EXPECT_FALSE(r.appendageRoute.get().has_value());
        EXPECT_FALSE(r.appendagePegOffset.get().has_value());
    }

    // =========================================================
    // STEP 2: Add MinQty and Route appendages, verify all three
    // =========================================================
    EXPECT_TRUE((o.appendageMinQty = 5u).has_value());
    EXPECT_TRUE((o.appendageRoute  = "INTL").has_value());

    // Fixed + 3 frames each (2 + 4)
    EXPECT_EQ(o.state_.total_size_, fixed_size + 3 * 6u);

    {
        size_t sz = o.state_.total_size_;
        Marshaler<OuchEnterOrder, true> r{{std::span<const char>(buffer, sz)}};

        auto firm = r.appendageFirm.get();
        ASSERT_TRUE(firm.has_value());
        EXPECT_EQ(*firm, "FIRM");

        auto minqty = r.appendageMinQty.get();
        ASSERT_TRUE(minqty.has_value());
        EXPECT_EQ(*minqty, 0x05u);

        auto route = r.appendageRoute.get();
        ASSERT_TRUE(route.has_value());
        EXPECT_EQ(*route, "INTL");

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
        EXPECT_EQ(*minqty, 0x05u);

        auto route = r.appendageRoute.get();
        ASSERT_TRUE(route.has_value());
        EXPECT_EQ(*route, "INTL");
    }

    // Removing again is a no-op (should succeed)
    EXPECT_TRUE(o.appendageFirm.remove().has_value());
    EXPECT_EQ(o.state_.total_size_, fixed_size + 2 * 6u);

    // =========================================================
    // STEP 4: Re-add Firm with different content, verify all
    //         three appendages again with correct values
    // =========================================================
    EXPECT_TRUE((o.appendageFirm = "ACME").has_value());

    // Back to 3 frames
    EXPECT_EQ(o.state_.total_size_, fixed_size + 3 * 6u);

    {
        size_t sz = o.state_.total_size_;
        Marshaler<OuchEnterOrder, true> r{{std::span<const char>(buffer, sz)}};

        // Firm is back with new content
        auto firm = r.appendageFirm.get();
        ASSERT_TRUE(firm.has_value());
        EXPECT_EQ(*firm, "ACME");

        // MinQty untouched
        auto minqty = r.appendageMinQty.get();
        ASSERT_TRUE(minqty.has_value());
        EXPECT_EQ(*minqty, 0x05u);

        // Route untouched
        auto route = r.appendageRoute.get();
        ASSERT_TRUE(route.has_value());
        EXPECT_EQ(*route, "INTL");
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

    EXPECT_TRUE((o.appendageFirm   = "XXXX").has_value());
    EXPECT_TRUE((o.appendageMinQty = 10u).has_value());
    EXPECT_TRUE((o.appendageRoute  = "NYSE").has_value());

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
        EXPECT_EQ(*firm, "XXXX");

        EXPECT_FALSE(r.appendageMinQty.get().has_value());

        auto route = r.appendageRoute.get();
        ASSERT_TRUE(route.has_value());
        EXPECT_EQ(*route, "NYSE");
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

    EXPECT_TRUE((o.appendageFirm  = "ABCD").has_value());
    EXPECT_TRUE((o.appendageRoute = "LAST").has_value());

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
        EXPECT_EQ(*firm, "ABCD");

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

    EXPECT_TRUE((o.appendageFirm   = "FFFF").has_value());
    EXPECT_TRUE((o.appendageMinQty = 3u).has_value());
    EXPECT_TRUE((o.appendageRoute  = "RRRR").has_value());

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
    EXPECT_TRUE((o.appendageRoute  = "NEWR").has_value());
    EXPECT_TRUE((o.appendageMinQty = 255u).has_value());
    EXPECT_TRUE((o.appendageFirm   = "NEWF").has_value());

    EXPECT_EQ(o.state_.total_size_, fixed_size + 18u);

    {
        size_t sz = o.state_.total_size_;
        Marshaler<OuchEnterOrder, true> r{{std::span<const char>(buffer, sz)}};

        auto firm = r.appendageFirm.get();
        ASSERT_TRUE(firm.has_value());
        EXPECT_EQ(*firm, "NEWF");

        auto minqty = r.appendageMinQty.get();
        ASSERT_TRUE(minqty.has_value());
        EXPECT_EQ(*minqty, 0xFFu);

        auto route = r.appendageRoute.get();
        ASSERT_TRUE(route.has_value());
        EXPECT_EQ(*route, "NEWR");
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
    EXPECT_TRUE((o.appendageHandleInst = 0x02).has_value());
    EXPECT_TRUE((o.appendageFirm       = "ORDR").has_value());

    size_t sz_after_two = o.state_.total_size_;

    // Step 2: Verify both
    {
        Marshaler<OuchReplaceOrder, true> r{{std::span<const char>(buffer, sz_after_two)}};
        auto hi = r.appendageHandleInst.get();
        ASSERT_TRUE(hi.has_value());
        EXPECT_EQ(*hi, 0x02u);

        auto firm = r.appendageFirm.get();
        ASSERT_TRUE(firm.has_value());
        EXPECT_EQ(*firm, "ORDR");
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
        EXPECT_EQ(*firm, "ORDR");
    }

    // Step 4: Re-add HandleInst with a different value
    EXPECT_TRUE((o.appendageHandleInst = 0x07).has_value());
    EXPECT_EQ(o.state_.total_size_, sz_after_two);

    {
        size_t sz = o.state_.total_size_;
        Marshaler<OuchReplaceOrder, true> r{{std::span<const char>(buffer, sz)}};

        auto hi = r.appendageHandleInst.get();
        ASSERT_TRUE(hi.has_value());
        EXPECT_EQ(*hi, 0x07u);  // new value

        auto firm = r.appendageFirm.get();
        ASSERT_TRUE(firm.has_value());
        EXPECT_EQ(*firm, "ORDR");
    }
}
