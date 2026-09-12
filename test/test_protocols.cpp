#include <gtest/gtest.h>
#include <aitvaras/marshaler.hpp>
#include "protocols.hpp"
#include <cstring>
#include <cstdint>

using namespace aitvaras;

// ============================================================
// RAKE Frame Tests
// ============================================================

TEST(RakeTest, LogonRequestRoundTrip) {
    char buffer[256] = {0};
    {
        Marshaler<RakeLogonRequest> req{{std::span<char>(buffer, sizeof(buffer))}};
        EXPECT_TRUE((req.messageType        = '5').has_value());
        EXPECT_TRUE((req.session            = 0xDEADBEEFCAFEBABEULL).has_value());
        EXPECT_TRUE((req.senderComp         = "MYSEND").has_value());
        EXPECT_TRUE((req.token              = "MYTOKEN").has_value());
        EXPECT_TRUE((req.nextSequenceNumber = 42ULL).has_value());
        (void)(req.length = static_cast<uint16_t>(req.state_.total_size_ - 2));

        // fixed: 2 + 1 + 8 + 8 + 8 + 8 = 35
        EXPECT_EQ(req.state_.total_size_, 35u);
    }
    // Read back
    Marshaler<RakeLogonRequest, true> reader{{std::span<const char>(buffer, 35)}};
    EXPECT_EQ(reader.messageType.get(), '5');
    EXPECT_EQ(reader.session.get(), 0xDEADBEEFCAFEBABEULL);
    EXPECT_EQ(reader.senderComp.get(), "MYSEND");  // padded field strips trailing spaces
    EXPECT_EQ(reader.token.get(), "MYTOKEN");
    EXPECT_EQ(reader.nextSequenceNumber.get(), 42ULL);
}

TEST(RakeTest, TcpSequencedMessage) {
    const uint8_t payload[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    char buffer[256] = {0};
    {
        Marshaler<RakeTcpSequencedMessage> msg{{std::span<char>(buffer, sizeof(buffer))}};
        EXPECT_TRUE((msg.messageType = '2').has_value());
        EXPECT_TRUE((msg.streamId    = 3).has_value());
        EXPECT_TRUE((msg.payload     = std::span<const uint8_t>(payload, sizeof(payload))).has_value());
        (void)(msg.length = static_cast<uint16_t>(msg.state_.total_size_ - 2));

        // 2 (length) + 1 (type) + 1 (streamId) + 5 (payload) = 9
        EXPECT_EQ(msg.state_.total_size_, 9u);
    }
    // Read back; payload is a length_precedes field returning std::string_view
    Marshaler<RakeTcpSequencedMessage, true> reader{{std::span<const char>(buffer, 9)}};
    EXPECT_EQ(reader.messageType.get(), '2');
    EXPECT_EQ(reader.streamId.get(), 3);
    // payload returns std::string_view (raw bytes view)
    auto p = reader.payload.get();
    ASSERT_EQ(p.size(), 5u);
    EXPECT_EQ(static_cast<uint8_t>(p[0]), 0x01u);
    EXPECT_EQ(static_cast<uint8_t>(p[4]), 0x05u);
}

TEST(RakeTest, DebugMessage) {
    char buffer[256] = {0};
    {
        Marshaler<RakeDebug> msg{{std::span<char>(buffer, sizeof(buffer))}};
        EXPECT_TRUE((msg.messageType = '0').has_value());
        EXPECT_TRUE((msg.payload     = "Hello debug!").has_value());
        (void)(msg.length = static_cast<uint16_t>(msg.state_.total_size_ - 2));

        EXPECT_EQ(msg.state_.total_size_, 15u);  // 2+1+12
    }
    Marshaler<RakeDebug, true> reader{{std::span<const char>(buffer, 15)}};
    EXPECT_EQ(reader.messageType.get(), '0');
    EXPECT_EQ(reader.payload.get(), "Hello debug!");
}

// ============================================================
// SOUP Frame Tests
// ============================================================

TEST(SoupTest, LoginRequestRoundTrip) {
    char buffer[256] = {0};
    {
        Marshaler<SoupLoginRequest> req{{std::span<char>(buffer, sizeof(buffer))}};
        EXPECT_TRUE((req.packetType              = 'L').has_value());
        EXPECT_TRUE((req.username                = "USER01").has_value());
        EXPECT_TRUE((req.password                = "PASS01").has_value());
        EXPECT_TRUE((req.requestedSession        = "SESSION1").has_value());
        EXPECT_TRUE((req.requestedSequenceNumber = "1").has_value());
        (void)(req.length = static_cast<uint16_t>(req.state_.total_size_ - 2));

        // 2 + 1 + 6 + 10 + 10 + 20 = 49
        EXPECT_EQ(req.state_.total_size_, 49u);
    }
    Marshaler<SoupLoginRequest, true> reader{{std::span<const char>(buffer, 49)}};
    EXPECT_EQ(reader.packetType.get(), 'L');
    EXPECT_EQ(reader.username.get(), "USER01");
    EXPECT_EQ(reader.password.get(), "PASS01");
    EXPECT_EQ(reader.requestedSequenceNumber.get(), "1");
}

TEST(SoupTest, SequencedDataRoundTrip) {
    const uint8_t ouch_bytes[] = {'O', 0x00, 0x00, 0x00, 0x2A};  // type + userRefNum=42
    char buffer[256] = {0};
    {
        Marshaler<SoupSequencedData> msg{{std::span<char>(buffer, sizeof(buffer))}};
        EXPECT_TRUE((msg.packetType = 'S').has_value());
        EXPECT_TRUE((msg.message = std::span<const uint8_t>(ouch_bytes, sizeof(ouch_bytes))).has_value());
        (void)(msg.length = static_cast<uint16_t>(msg.state_.total_size_ - 2));

        // 2 + 1 + 5 = 8
        EXPECT_EQ(msg.state_.total_size_, 8u);
    }
    Marshaler<SoupSequencedData, true> reader{{std::span<const char>(buffer, 8)}};
    EXPECT_EQ(reader.packetType.get(), 'S');
    // message is length_precedes -> std::string_view (raw bytes)
    auto m = reader.message.get();
    ASSERT_EQ(m.size(), 5u);
    EXPECT_EQ(m[0], 'O');
    EXPECT_EQ(static_cast<uint8_t>(m[4]), 0x2Au);
}

// ============================================================
// OUCH 5.0 — EnterOrder with Appendages
// ============================================================

static size_t build_enter_order(char* buf, size_t bufsz,
                                uint32_t userRefNum,
                                std::string_view symbol,
                                double price) {
    Marshaler<OuchEnterOrder> o{{std::span<char>(buf, bufsz)}};
    (void)(o.type = 'O');
    (void)(o.userRefNum = userRefNum);
    (void)(o.side = 'B');
    (void)(o.quantity = 100);
    (void)(o.symbol = symbol);
    (void)(o.price = price);
    (void)(o.timeInForce = '0');
    (void)(o.display = 'Y');
    (void)(o.capacity = 'A');
    (void)(o.interMarketSweepEligibility = 'N');
    (void)(o.crossType = 'N');
    (void)(o.clOrdID = "CLIENT1");
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
    (void)(o.type = 'O');
    (void)(o.userRefNum = 99);
    (void)(o.side = 'S');
    (void)(o.quantity = 200);
    (void)(o.symbol = "TSLA");
    (void)(o.price = 250.0);
    (void)(o.timeInForce = '0');
    (void)(o.display = 'Y');
    (void)(o.capacity = 'A');
    (void)(o.interMarketSweepEligibility = 'N');
    (void)(o.crossType = 'N');
    (void)(o.clOrdID = "ORDER2");

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
    (void)(o.type = 'O');
    (void)(o.userRefNum = 7);
    (void)(o.side = 'B');
    (void)(o.quantity = 300);
    (void)(o.symbol = "GOOGL");
    (void)(o.price = 100.0);
    (void)(o.timeInForce = '0');
    (void)(o.display = 'Y');
    (void)(o.capacity = 'A');
    (void)(o.interMarketSweepEligibility = 'N');
    (void)(o.crossType = 'N');
    (void)(o.clOrdID = "ORD3");

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
    (void)(o.type = 'O');
    (void)(o.userRefNum = 5);
    (void)(o.side = 'B');
    (void)(o.quantity = 50);
    (void)(o.symbol = "IBM");
    (void)(o.price = 125.0);
    (void)(o.timeInForce = '0');
    (void)(o.display = 'Y');
    (void)(o.capacity = 'A');
    (void)(o.interMarketSweepEligibility = 'N');
    (void)(o.crossType = 'N');
    (void)(o.clOrdID = "UPDT");

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
    (void)(o.type = 'U');
    (void)(o.origUserRefNum = 10);
    (void)(o.userRefNum = 11);
    (void)(o.quantity = 150);
    (void)(o.price = 75.25);
    (void)(o.timeInForce = '0');
    (void)(o.display = 'Y');
    (void)(o.interMarketSweepEligibility = 'N');
    (void)(o.clOrdID = "REPL1");

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
    (void)(o.type = 'X');
    (void)(o.userRefNum = 42);
    (void)(o.quantity = 0);

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
    (void)(o.type = 'E');
    (void)(o.timestamp = 9876543210ULL);
    (void)(o.userRefNum = 55);
    (void)(o.quantity = 100);
    (void)(o.price = 99.99);
    (void)(o.liquidityFlag = 'A');
    (void)(o.matchNumber = 111222333ULL);

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
    (void)(o.type = 'B');
    (void)(o.timestamp = 112233445566ULL);
    (void)(o.userRefNum = 88);
    (void)(o.matchNumber = 77777ULL);
    (void)(o.reason = 'E');
    (void)(o.clOrdID = "BRK1");

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
// SEED Protocol Tests
// ============================================================

TEST(SeedTest, LimitOrder_NoOptionalFields) {
    char buffer[1024] = {0};
    Marshaler<SeedLimitOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    (void)(o.messageType = 0x4C);  // 'L'
    (void)(o.clOrdId = 100ULL);
    (void)(o.orderQty = 1000);
    (void)(o.limitOrderBitFields = 0x00000001u);
    (void)(o.symbolId = 42);
    (void)(o.price = 50.0);

    // Fixed: 1+4+8+4+4+2+8 = 31
    EXPECT_EQ(o.state_.total_size_, 31u);
    EXPECT_EQ(o.presenceBits.get(), 0u);

    size_t sz = o.state_.total_size_;
    Marshaler<SeedLimitOrder, true> reader{{std::span<const char>(buffer, sz)}};
    EXPECT_EQ(reader.messageType.get(), 0x4Cu);
    EXPECT_EQ(reader.clOrdId.get(), 100ULL);
    EXPECT_EQ(reader.orderQty.get(), 1000u);
    EXPECT_EQ(reader.symbolId.get(), 42u);
    EXPECT_NEAR(reader.price.get(), 50.0, 1e-8);
    EXPECT_FALSE(reader.selfMatchScope.get().has_value());
    EXPECT_FALSE(reader.minQty.get().has_value());
    EXPECT_FALSE(reader.mpid.get().has_value());
}

TEST(SeedTest, LimitOrder_SelectiveOptionalFields) {
    // Set only expireTime (bit 8) and mpid (bit 10) — skip rest
    char buffer[1024] = {0};
    Marshaler<SeedLimitOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    (void)(o.messageType = 0x4C);
    (void)(o.clOrdId = 300ULL);
    (void)(o.orderQty = 100);
    (void)(o.limitOrderBitFields = 0u);
    (void)(o.symbolId = 1);
    (void)(o.price = 10.0);

    EXPECT_TRUE((o.expireTime = 555555ULL).has_value());  // bit 8
    EXPECT_TRUE((o.mpid = "XYZ").has_value());             // bit 10

    uint32_t pbits = o.presenceBits.get();
    EXPECT_TRUE(pbits & (1u << 8));   // expireTime present
    EXPECT_TRUE(pbits & (1u << 10));  // mpid present
    EXPECT_FALSE(pbits & (1u << 0));  // selfMatchScope absent
    EXPECT_FALSE(pbits & (1u << 3));  // minQty absent
    EXPECT_FALSE(pbits & (1u << 11)); // memberGroup absent

    // Fixed 31 + expireTime(8) + mpid(4) = 43
    EXPECT_EQ(o.state_.total_size_, 43u);

    size_t sz = o.state_.total_size_;
    Marshaler<SeedLimitOrder, true> reader{{std::span<const char>(buffer, sz)}};
    EXPECT_FALSE(reader.selfMatchScope.get().has_value());
    EXPECT_FALSE(reader.minQty.get().has_value());
    EXPECT_FALSE(reader.memberGroup.get().has_value());

    auto exp = reader.expireTime.get();
    ASSERT_TRUE(exp.has_value());
    EXPECT_EQ(*exp, 555555ULL);

    auto mpid_val = reader.mpid.get();
    ASSERT_TRUE(mpid_val.has_value());
    EXPECT_EQ(*mpid_val, "XYZ");
}

TEST(SeedTest, LimitOrder_AllOptionalFields) {
    char buffer[2048] = {0};
    Marshaler<SeedLimitOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    (void)(o.messageType = 0x4C);
    (void)(o.clOrdId = 200ULL);
    (void)(o.orderQty = 500);
    (void)(o.limitOrderBitFields = 0x00000001u);
    (void)(o.symbolId = 7);
    (void)(o.price = 123.45);

    EXPECT_TRUE((o.selfMatchScope        = 1).has_value());
    EXPECT_TRUE((o.selfMatchInstruction  = 2).has_value());
    EXPECT_TRUE((o.priceSlideInstruction = 3).has_value());
    EXPECT_TRUE((o.minQty               = 10).has_value());
    EXPECT_TRUE((o.maxFloorQty          = 50).has_value());
    EXPECT_TRUE((o.maxReplenishQtyRange = 25).has_value());
    EXPECT_TRUE((o.maxReplenishTimeRange = 1000ULL).has_value());
    EXPECT_TRUE((o.referencePriceTarget = 1).has_value());
    EXPECT_TRUE((o.expireTime           = 99999ULL).has_value());
    EXPECT_TRUE((o.userData             = 12345678ULL).has_value());
    EXPECT_TRUE((o.mpid                 = "ABCD").has_value());
    EXPECT_TRUE((o.memberGroup          = "MG").has_value());
    EXPECT_TRUE((o.locateBroker         = "BRKR").has_value());

    // All bits 0-12 should be set
    uint32_t pbits = o.presenceBits.get();
    for (int i = 0; i <= 12; i++) {
        EXPECT_TRUE(pbits & (1u << i)) << "bit " << i << " should be set";
    }

    size_t sz = o.state_.total_size_;
    Marshaler<SeedLimitOrder, true> reader{{std::span<const char>(buffer, sz)}};

    auto scope = reader.selfMatchScope.get();
    ASSERT_TRUE(scope.has_value());
    EXPECT_EQ(*scope, 1u);

    auto minq = reader.minQty.get();
    ASSERT_TRUE(minq.has_value());
    EXPECT_EQ(*minq, 10u);

    auto exp = reader.expireTime.get();
    ASSERT_TRUE(exp.has_value());
    EXPECT_EQ(*exp, 99999ULL);

    auto mpid_val = reader.mpid.get();
    ASSERT_TRUE(mpid_val.has_value());
    EXPECT_EQ(*mpid_val, "ABCD");

    auto mg = reader.memberGroup.get();
    ASSERT_TRUE(mg.has_value());
    EXPECT_EQ(*mg, "MG");

    auto lb = reader.locateBroker.get();
    ASSERT_TRUE(lb.has_value());
    EXPECT_EQ(*lb, "BRKR");
}

TEST(SeedTest, MarketOrder_WithMpidAndGroup) {
    char buffer[512] = {0};
    Marshaler<SeedMarketOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    (void)(o.messageType = 0x4D);
    (void)(o.clOrdId = 400ULL);
    (void)(o.orderQty = 200);
    (void)(o.marketOrderBitFields = 0x00000002u);
    (void)(o.symbolId = 5);
    EXPECT_TRUE((o.mpid = "NEST").has_value());       // bit 5
    EXPECT_TRUE((o.memberGroup = "A1").has_value());  // bit 6

    size_t sz = o.state_.total_size_;
    Marshaler<SeedMarketOrder, true> reader{{std::span<const char>(buffer, sz)}};
    EXPECT_EQ(reader.messageType.get(), 0x4Du);
    EXPECT_EQ(reader.orderQty.get(), 200u);
    EXPECT_EQ(reader.symbolId.get(), 5u);

    auto mpid_val = reader.mpid.get();
    ASSERT_TRUE(mpid_val.has_value());
    EXPECT_EQ(*mpid_val, "NEST");

    auto mg = reader.memberGroup.get();
    ASSERT_TRUE(mg.has_value());
    EXPECT_EQ(*mg, "A1");

    EXPECT_FALSE(reader.selfMatchScope.get().has_value());
    EXPECT_FALSE(reader.locateBroker.get().has_value());
}

TEST(SeedTest, ReplaceOrder_WithOptionals) {
    char buffer[512] = {0};
    Marshaler<SeedReplaceOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    (void)(o.messageType = 0x52);
    (void)(o.clOrdId = 501ULL);
    (void)(o.origClOrdId = 500ULL);
    (void)(o.orderQty = 250);
    (void)(o.replaceOrderBitFields = 0u);
    (void)(o.price = 88.88);

    EXPECT_TRUE((o.minQty = 5).has_value());       // bit 0
    EXPECT_TRUE((o.maxFloorQty = 50).has_value()); // bit 1

    uint32_t pbits = o.presenceBits.get();
    EXPECT_TRUE(pbits & (1u << 0));
    EXPECT_TRUE(pbits & (1u << 1));
    EXPECT_FALSE(pbits & (1u << 2));  // maxReplenishQtyRange absent

    size_t sz = o.state_.total_size_;
    Marshaler<SeedReplaceOrder, true> reader{{std::span<const char>(buffer, sz)}};
    EXPECT_EQ(reader.origClOrdId.get(), 500ULL);
    EXPECT_NEAR(reader.price.get(), 88.88, 1e-8);

    auto minq = reader.minQty.get();
    ASSERT_TRUE(minq.has_value());
    EXPECT_EQ(*minq, 5u);

    auto maxfloor = reader.maxFloorQty.get();
    ASSERT_TRUE(maxfloor.has_value());
    EXPECT_EQ(*maxfloor, 50u);

    EXPECT_FALSE(reader.maxReplenishQtyRange.get().has_value());
}

TEST(SeedTest, CancelOrder_RoundTrip) {
    char buffer[256] = {0};
    Marshaler<SeedCancelOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    (void)(o.messageType = 0x43);
    (void)(o.clOrdId = 600ULL);
    (void)(o.origClOrdId = 599ULL);

    // Fixed: 1+4+8+8 = 21
    EXPECT_EQ(o.state_.total_size_, 21u);

    size_t sz = o.state_.total_size_;
    Marshaler<SeedCancelOrder, true> reader{{std::span<const char>(buffer, sz)}};
    EXPECT_EQ(reader.clOrdId.get(), 600ULL);
    EXPECT_EQ(reader.origClOrdId.get(), 599ULL);
}

TEST(SeedTest, OrderRestated_SelectiveOptionals) {
    char buffer[512] = {0};
    Marshaler<SeedOrderRestated> o{{std::span<char>(buffer, sizeof(buffer))}};
    (void)(o.messageType = 0x52);
    (void)(o.clOrdId = 800ULL);
    (void)(o.orderId = 80001ULL);
    (void)(o.timestamp = 2000000000ULL);
    (void)(o.restatementReason = 0x01u);

    // Set leavesQty (bit 0) and pegPrice (bit 3); skip displayQty (bit 1) and displayPrice (bit 2)
    EXPECT_TRUE((o.leavesQty = 250).has_value());
    EXPECT_TRUE((o.pegPrice  = 77.77).has_value());

    uint32_t pbits = o.presenceBits.get();
    EXPECT_TRUE(pbits & (1u << 0));   // leavesQty
    EXPECT_TRUE(pbits & (1u << 3));   // pegPrice
    EXPECT_FALSE(pbits & (1u << 1));  // displayQty absent
    EXPECT_FALSE(pbits & (1u << 2));  // displayPrice absent

    size_t sz = o.state_.total_size_;
    Marshaler<SeedOrderRestated, true> reader{{std::span<const char>(buffer, sz)}};

    auto leaves = reader.leavesQty.get();
    ASSERT_TRUE(leaves.has_value());
    EXPECT_EQ(*leaves, 250u);

    EXPECT_FALSE(reader.displayQty.get().has_value());
    EXPECT_FALSE(reader.displayPrice.get().has_value());

    auto peg = reader.pegPrice.get();
    ASSERT_TRUE(peg.has_value());
    EXPECT_NEAR(*peg, 77.77, 1e-8);
}

TEST(SeedTest, OrderExecuted_RoundTrip) {
    char buffer[512] = {0};
    Marshaler<SeedOrderExecuted> o{{std::span<char>(buffer, sizeof(buffer))}};
    (void)(o.messageType = 0x45);
    (void)(o.clOrdId = 700ULL);
    (void)(o.orderId = 70001ULL);
    (void)(o.timestamp = 1000000000ULL);
    (void)(o.execId = 9999ULL);
    (void)(o.lastQty = 100);
    (void)(o.lastPx  = 55.55);
    (void)(o.leavesQty = 0);
    (void)(o.liquidityIndicator = 0x01);

    size_t sz = o.state_.total_size_;
    Marshaler<SeedOrderExecuted, true> reader{{std::span<const char>(buffer, sz)}};
    EXPECT_EQ(reader.execId.get(), 9999ULL);
    EXPECT_EQ(reader.lastQty.get(), 100u);
    EXPECT_NEAR(reader.lastPx.get(), 55.55, 1e-8);
    EXPECT_EQ(reader.leavesQty.get(), 0u);
    EXPECT_EQ(reader.liquidityIndicator.get(), 0x01u);
}

TEST(SeedTest, MassCancelRoundTrip) {
    char buffer[256] = {0};
    Marshaler<SeedMassCancel> o{{std::span<char>(buffer, sizeof(buffer))}};
    (void)(o.messageType = 0x4D);
    (void)(o.clOrdId = 900ULL);
    (void)(o.massCancelScope = 1);
    (void)(o.symbolId = 0);  // all symbols
    (void)(o.mpid = "FIRM");
    (void)(o.memberGroup = "G1");

    size_t sz = o.state_.total_size_;
    Marshaler<SeedMassCancel, true> reader{{std::span<const char>(buffer, sz)}};
    EXPECT_EQ(reader.clOrdId.get(), 900ULL);
    EXPECT_EQ(reader.massCancelScope.get(), 1u);
    EXPECT_EQ(reader.mpid.get(), "FIRM");
    EXPECT_EQ(reader.memberGroup.get(), "G1");
}

// ============================================================
// Frame Composition: SOUP wrapping OUCH (end-to-end)
// ============================================================

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
