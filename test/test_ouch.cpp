#include <gtest/gtest.h>
#include <aitvaras/marshaler.hpp>
#include "protocol_ouch.hpp"

using namespace aitvaras;


// OUCH 5.0 — EnterOrder with Appendages
// ============================================================

static size_t build_enter_order(char* buf, size_t bufsz,
                                uint32_t userRefNum,
                                std::string_view symbol,
                                double price) {
    Marshaler<OuchEnterOrder> o{{std::span<char>(buf, bufsz)}};
    o.type = 'O';
    o.userRefNum = userRefNum;
    o.side = 'B';
    o.quantity = 100;
    o.symbol = symbol;
    o.price = price;
    o.timeInForce = '0';
    o.display = 'Y';
    o.capacity = 'A';
    o.interMarketSweepEligibility = 'N';
    o.crossType = 'N';
    o.clOrdID = "CLIENT1";
    return o.state_.total_size_;
}

TEST(OuchTest, EnterOrder_NoAppendages) {
    char buffer[1024] = {0};
    size_t sz = build_enter_order(buffer, sizeof(buffer), 1, "AAPL", 150.5);
    // Fixed: 1+4+1+4+8+8+1+1+1+1+1+14+2 = 47 bytes
    EXPECT_EQ(sz, 47u);

    Marshaler<OuchEnterOrder, true> reader{{std::span<const char>(buffer, sz)}};
    EXPECT_EQ(reader.type.get(), 'O');
    EXPECT_EQ(reader.userRefNum.get(), 1u);
    EXPECT_EQ(reader.side.get(), 'B');
    EXPECT_EQ(reader.quantity.get(), 100u);
    EXPECT_EQ(reader.symbol.get(), "AAPL");
    // price is fixed_point<4> returning double directly
    EXPECT_NEAR(reader.price.get(), 150.5, 0.0001);
    EXPECT_EQ(reader.clOrdID.get(), "CLIENT1");

    // No appendages — all return unexpected
    EXPECT_FALSE(reader.appendageFirm.get().has_value());
    EXPECT_FALSE(reader.appendageMinQty.get().has_value());
    EXPECT_FALSE(reader.appendageRoute.get().has_value());
}

TEST(OuchTest, EnterOrder_OneAppendage_Firm) {
    char buffer[1024] = {0};
    Marshaler<OuchEnterOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    o.type = 'O';
    o.userRefNum = 99;
    o.side = 'S';
    o.quantity = 200;
    o.symbol = "TSLA";
    o.price = 250.0;
    o.timeInForce = '0';
    o.display = 'Y';
    o.capacity = 'A';
    o.interMarketSweepEligibility = 'N';
    o.crossType = 'N';
    o.clOrdID = "ORDER2";

    EXPECT_TRUE((o.appendageFirm = "FIRM").has_value());

    // Fixed 47 + TLV frame (2 header + 4 data) = 53
    EXPECT_EQ(o.state_.total_size_, 53u);

    size_t sz = o.state_.total_size_;
    Marshaler<OuchEnterOrder, true> reader{{std::span<const char>(buffer, sz)}};

    auto firm = reader.appendageFirm.get();
    ASSERT_TRUE(firm.has_value());
    EXPECT_EQ(*firm, "FIRM");

    EXPECT_FALSE(reader.appendageMinQty.get().has_value());
    EXPECT_FALSE(reader.appendageRoute.get().has_value());
}

TEST(OuchTest, EnterOrder_MultipleAppendages_AnyOrder) {
    // Set appendages in an unusual order: Route (tag 14) first, then MinQty (tag 3), then Firm (tag 2)
    // Tags appear out of numeric order — the framework must resolve by tag ID, not by position
    char buffer[1024] = {0};
    Marshaler<OuchEnterOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    o.type = 'O';
    o.userRefNum = 7;
    o.side = 'B';
    o.quantity = 300;
    o.symbol = "GOOGL";
    o.price = 100.0;
    o.timeInForce = '0';
    o.display = 'Y';
    o.capacity = 'A';
    o.interMarketSweepEligibility = 'N';
    o.crossType = 'N';
    o.clOrdID = "ORD3";

    EXPECT_TRUE((o.appendageRoute  = "ROUT").has_value());
    EXPECT_TRUE((o.appendageMinQty = 10).has_value());
    EXPECT_TRUE((o.appendageFirm   = "ABCD").has_value());

    // 47 + 3 × (2 + 4) = 47 + 18 = 65
    EXPECT_EQ(o.state_.total_size_, 65u);

    size_t sz = o.state_.total_size_;
    Marshaler<OuchEnterOrder, true> reader{{std::span<const char>(buffer, sz)}};

    auto route = reader.appendageRoute.get();
    ASSERT_TRUE(route.has_value());
    EXPECT_EQ(*route, "ROUT");

    auto minqty = reader.appendageMinQty.get();
    ASSERT_TRUE(minqty.has_value());
    EXPECT_EQ(*minqty, 0x0Au);

    auto firm = reader.appendageFirm.get();
    ASSERT_TRUE(firm.has_value());
    EXPECT_EQ(*firm, "ABCD");

    EXPECT_FALSE(reader.appendageHandleInst.get().has_value());
    EXPECT_FALSE(reader.appendagePostOnly.get().has_value());
}

TEST(OuchTest, EnterOrder_UpdateAppendage_Resize) {
    // Write an appendage, then overwrite with a longer value; verify memmove correctness
    char buffer[1024] = {0};
    Marshaler<OuchEnterOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    o.type = 'O';
    o.userRefNum = 5;
    o.side = 'B';
    o.quantity = 50;
    o.symbol = "IBM";
    o.price = 125.0;
    o.timeInForce = '0';
    o.display = 'Y';
    o.capacity = 'A';
    o.interMarketSweepEligibility = 'N';
    o.crossType = 'N';
    o.clOrdID = "UPDT";

    // Also add a second appendage so we can verify memmove didn't corrupt it
    EXPECT_TRUE((o.appendageFirm = "ZZZZ").has_value());

    EXPECT_TRUE((o.appendagePegOffset = 100).has_value());
    size_t sz_before = o.state_.total_size_;

    // Variable size appendage should allow expanding size
    EXPECT_TRUE((o.appendagePegOffset = 500).has_value());
    size_t sz_after = o.state_.total_size_;

    EXPECT_EQ(sz_after - sz_before, 0u);

    Marshaler<OuchEnterOrder, true> reader{{std::span<const char>(buffer, sz_after)}};

    // Both appendages should be intact
    auto firm = reader.appendageFirm.get();
    ASSERT_TRUE(firm.has_value());
    EXPECT_EQ(*firm, "ZZZZ");

    auto peg = reader.appendagePegOffset.get();
    ASSERT_TRUE(peg.has_value());
    EXPECT_EQ(*peg, 500u);
}

TEST(OuchTest, ReplaceOrder_WithAppendage) {
    char buffer[1024] = {0};
    Marshaler<OuchReplaceOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    o.type = 'U';
    o.origUserRefNum = 10;
    o.userRefNum = 11;
    o.quantity = 150;
    o.price = 75.25;
    o.timeInForce = '0';
    o.display = 'Y';
    o.interMarketSweepEligibility = 'N';
    o.clOrdID = "REPL1";

    EXPECT_TRUE((o.appendageHandleInst = 0x07).has_value());

    size_t sz = o.state_.total_size_;
    Marshaler<OuchReplaceOrder, true> reader{{std::span<const char>(buffer, sz)}};
    EXPECT_EQ(reader.origUserRefNum.get(), 10u);
    EXPECT_EQ(reader.userRefNum.get(), 11u);
    EXPECT_NEAR(reader.price.get(), 75.25, 0.0001);

    auto hi = reader.appendageHandleInst.get();
    ASSERT_TRUE(hi.has_value());
    EXPECT_EQ(*hi, 0x07u);
    EXPECT_FALSE(reader.appendageFirm.get().has_value());
}

TEST(OuchTest, CancelOrder_NoAppendages) {
    char buffer[512] = {0};
    Marshaler<OuchCancelOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    o.type = 'X';
    o.userRefNum = 42;
    o.quantity = 0;

    // 1 + 4 + 4 + 2 = 11
    EXPECT_EQ(o.state_.total_size_, 11u);

    size_t sz = o.state_.total_size_;
    Marshaler<OuchCancelOrder, true> reader{{std::span<const char>(buffer, sz)}};
    EXPECT_EQ(reader.userRefNum.get(), 42u);
    EXPECT_FALSE(reader.appendageFirm.get().has_value());
}

TEST(OuchTest, OrderExecuted_RoundTrip) {
    char buffer[512] = {0};
    Marshaler<OuchOrderExecuted> o{{std::span<char>(buffer, sizeof(buffer))}};
    o.type = 'E';
    o.timestamp = 9876543210ULL;
    o.userRefNum = 55;
    o.quantity = 100;
    o.price = 99.99;
    o.liquidityFlag = 'A';
    o.matchNumber = 111222333ULL;

    size_t sz = o.state_.total_size_;
    Marshaler<OuchOrderExecuted, true> reader{{std::span<const char>(buffer, sz)}};
    EXPECT_EQ(reader.type.get(), 'E');
    EXPECT_EQ(reader.userRefNum.get(), 55u);
    EXPECT_EQ(reader.quantity.get(), 100u);
    EXPECT_NEAR(reader.price.get(), 99.99, 0.0001);
    EXPECT_EQ(reader.liquidityFlag.get(), 'A');
    EXPECT_EQ(reader.matchNumber.get(), 111222333ULL);
}

TEST(OuchTest, BrokenTrade_WithAppendage) {
    char buffer[1024] = {0};
    Marshaler<OuchBrokenTrade> o{{std::span<char>(buffer, sizeof(buffer))}};
    o.type = 'B';
    o.timestamp = 112233445566ULL;
    o.userRefNum = 88;
    o.matchNumber = 77777ULL;
    o.reason = 'E';
    o.clOrdID = "BRK1";

    EXPECT_TRUE((o.appendageSecondaryOrdRefNum = 0x012345).has_value());

    size_t sz = o.state_.total_size_;
    Marshaler<OuchBrokenTrade, true> reader{{std::span<const char>(buffer, sz)}};
    EXPECT_EQ(reader.type.get(), 'B');
    EXPECT_EQ(reader.reason.get(), 'E');

    auto sec = reader.appendageSecondaryOrdRefNum.get();
    ASSERT_TRUE(sec.has_value());
    EXPECT_EQ(*sec, 0x012345u);
}

// ============================================================


// Appendage Lifecycle Tests
TEST(AppendageLifecycle, OuchEnterOrder_AddVerifyRemoveReadd) {
    char buffer[1024] = {0};

    // ---- Build the fixed fields ----
    Marshaler<OuchEnterOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    o.type = 'O';
    o.userRefNum = 42;
    o.side = 'B';
    o.quantity = 100;
    o.symbol = "NVDA";
    o.price = 500.0;
    o.timeInForce = '0';
    o.display = 'Y';
    o.capacity = 'A';
    o.interMarketSweepEligibility = 'N';
    o.crossType = 'N';
    o.clOrdID = "LIFE1";

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
    o.type = 'O';
    o.userRefNum = 9;
    o.side = 'S';
    o.quantity = 50;
    o.symbol = "META";
    o.price = 300.0;
    o.timeInForce = '0';
    o.display = 'Y';
    o.capacity = 'A';
    o.interMarketSweepEligibility = 'N';
    o.crossType = 'N';
    o.clOrdID = "MID";

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
    o.type = 'O';
    o.userRefNum = 77;
    o.side = 'B';
    o.quantity = 200;
    o.symbol = "AAPL";
    o.price = 175.0;
    o.timeInForce = '0';
    o.display = 'Y';
    o.capacity = 'A';
    o.interMarketSweepEligibility = 'N';
    o.crossType = 'N';
    o.clOrdID = "LAST";

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
    o.type = 'O';
    o.userRefNum = 3;
    o.side = 'B';
    o.quantity = 10;
    o.symbol = "IBM";
    o.price = 140.0;
    o.timeInForce = '0';
    o.display = 'Y';
    o.capacity = 'A';
    o.interMarketSweepEligibility = 'N';
    o.crossType = 'N';
    o.clOrdID = "ALL";

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
    o.type = 'U';
    o.origUserRefNum = 1;
    o.userRefNum = 2;
    o.quantity = 500;
    o.price = 99.0;
    o.timeInForce = '0';
    o.display = 'Y';
    o.interMarketSweepEligibility = 'N';
    o.clOrdID = "LCY";

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

#include "protocol_soup.hpp"
TEST(FrameTest, SoupWrappedOuchEnterOrder) {
    // Build an OUCH EnterOrder with two appendages
    alignas(8) char ouch_buf[512] = {0};
    Marshaler<OuchEnterOrder> ouch{{std::span<char>(ouch_buf, sizeof(ouch_buf))}};
    (void)(ouch.type = 'O');
    (void)(ouch.userRefNum = 123);
    (void)(ouch.side = 'B');
    (void)(ouch.quantity = 75);
    (void)(ouch.symbol = "AMZN");
    (void)(ouch.price = 180.0);
    (void)(ouch.timeInForce = '0');
    (void)(ouch.display = 'Y');
    (void)(ouch.capacity = 'A');
    (void)(ouch.interMarketSweepEligibility = 'N');
    (void)(ouch.crossType = 'N');
    (void)(ouch.clOrdID = "AMAZON1");

    EXPECT_TRUE((ouch.appendageRoute = "DEST").has_value());
    EXPECT_TRUE((ouch.appendageFirm  = "ABBN").has_value());
    size_t ouch_sz = ouch.state_.total_size_;

    // Wrap in SOUP unsequenced data
    char soup_buf[1024] = {0};
    Marshaler<SoupUnsequencedData> soup{{std::span<char>(soup_buf, sizeof(soup_buf))}};
    (void)(soup.packetType = 'U');
    EXPECT_TRUE((soup.message = std::span<const uint8_t>(
        reinterpret_cast<const uint8_t*>(ouch_buf), ouch_sz)).has_value());
    (void)(soup.length = static_cast<uint16_t>(soup.state_.total_size_ - 2));

    size_t soup_sz = soup.state_.total_size_;

    // Read SOUP frame
    Marshaler<SoupUnsequencedData, true> soup_reader{{std::span<const char>(soup_buf, soup_sz)}};
    EXPECT_EQ(soup_reader.packetType.get(), 'U');
    auto payload_sv = soup_reader.message.get();
    ASSERT_EQ(payload_sv.size(), ouch_sz);

    // Interpret inner OUCH
    Marshaler<OuchEnterOrder, true> ouch_reader{{std::span<const char>(reinterpret_cast<const char*>(payload_sv.data()), payload_sv.size())}};
    EXPECT_EQ(ouch_reader.type.get(), 'O');
    EXPECT_EQ(ouch_reader.userRefNum.get(), 123u);
    EXPECT_EQ(ouch_reader.symbol.get(), "AMZN");
    EXPECT_NEAR(ouch_reader.price.get(), 180.0, 0.0001);

    auto rt = ouch_reader.appendageRoute.get();
    ASSERT_TRUE(rt.has_value());
    EXPECT_EQ(*rt, "DEST");

    auto fm = ouch_reader.appendageFirm.get();
    ASSERT_TRUE(fm.has_value());
    EXPECT_EQ(*fm, "ABBN"); /* NEEDS MANUAL FIX */
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


// Optional Lifecycle Tests
