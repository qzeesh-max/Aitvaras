#include <gtest/gtest.h>
#include <aitvaras/marshaler.hpp>
#include "protocol_seed.hpp"

using namespace aitvaras;


// SEED Protocol Tests
// ============================================================

TEST(SeedTest, LimitOrder_NoOptionalFields) {
    char buffer[1024] = {0};
    Marshaler<SeedLimitOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    o.messageType = 0x4C;  // 'L'
    o.clOrdId = 100ULL;
    o.orderQty = 1000;
    o.limitOrderBitFields = 0x00000001u;
    o.symbolId = 42;
    o.price = 50.0;

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
    o.messageType = 0x4C;
    o.clOrdId = 300ULL;
    o.orderQty = 100;
    o.limitOrderBitFields = 0u;
    o.symbolId = 1;
    o.price = 10.0;

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
    o.messageType = 0x4C;
    o.clOrdId = 200ULL;
    o.orderQty = 500;
    o.limitOrderBitFields = 0x00000001u;
    o.symbolId = 7;
    o.price = 123.45;

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
    o.messageType = 0x4D;
    o.clOrdId = 400ULL;
    o.orderQty = 200;
    o.marketOrderBitFields = 0x00000002u;
    o.symbolId = 5;
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
    o.messageType = 0x52;
    o.clOrdId = 501ULL;
    o.origClOrdId = 500ULL;
    o.orderQty = 250;
    o.replaceOrderBitFields = 0u;
    o.price = 88.88;

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
    o.messageType = 0x43;
    o.clOrdId = 600ULL;
    o.origClOrdId = 599ULL;

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
    o.messageType = 0x52;
    o.clOrdId = 800ULL;
    o.orderId = 80001ULL;
    o.timestamp = 2000000000ULL;
    o.restatementReason = 0x01u;

    // Set leavesQty (bit 0) and pegPrice (bit 3); skip displayQty (bit 1) and displayPrice (bit 2)
    o.leavesQty = 250;
    o.pegPrice = 77.77;

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
    o.messageType = 0x45;
    o.clOrdId = 700ULL;
    o.orderId = 70001ULL;
    o.timestamp = 1000000000ULL;
    o.execId = 9999ULL;
    o.lastQty = 100;
    o.lastPx  = 55.55;
    o.leavesQty = 0;
    o.liquidityIndicator = 0x01;

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
    o.messageType = 0x4D;
    o.clOrdId = 900ULL;
    o.massCancelScope = 1;
    o.symbolId = 0;  // all symbols
    o.mpid = "FIRM";
    o.memberGroup = "G1";

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

TEST(OptionalLifecycle, SeedLimitOrder_AddVerifyRemoveReadd) {
    char buffer[1024] = {0};

    Marshaler<SeedLimitOrder> o{{std::span<char>(buffer, sizeof(buffer))}};
    o.messageType = 'O';
    o.clOrdId = 123456789ULL;
    o.orderQty = 500;
    o.limitOrderBitFields = 0;
    o.symbolId = 1;
    o.price = 150.0;

    const size_t fixed_size = 31;
    EXPECT_EQ(o.state_.total_size_, fixed_size);

    // Step 1: Add maxFloorQty (4 bytes)
    EXPECT_TRUE((o.maxFloorQty = 100).has_value());

    size_t sz_after_one = o.state_.total_size_;
    EXPECT_EQ(sz_after_one, fixed_size + 4u);

    {
        Marshaler<SeedLimitOrder, true> r{{std::span<const char>(buffer, sz_after_one)}};
        auto maxf = r.maxFloorQty.get();
        ASSERT_TRUE(maxf.has_value());
        EXPECT_EQ(*maxf, 100u);
    }

    // Step 2: Add mpid (4 bytes)
    EXPECT_TRUE((o.mpid = "BAML").has_value());

    size_t sz_after_two = o.state_.total_size_;
    EXPECT_EQ(sz_after_two, sz_after_one + 4u);

    {
        Marshaler<SeedLimitOrder, true> r{{std::span<const char>(buffer, sz_after_two)}};
        auto mpid = r.mpid.get();
        ASSERT_TRUE(mpid.has_value());
        EXPECT_EQ(*mpid, "BAML");

        auto maxf = r.maxFloorQty.get();
        ASSERT_TRUE(maxf.has_value());
        EXPECT_EQ(*maxf, 100u);
    }

    // Step 3: Remove maxFloorQty
    EXPECT_TRUE(o.maxFloorQty.remove().has_value());
    EXPECT_EQ(o.state_.total_size_, sz_after_two - 4u);

    {
        size_t sz = o.state_.total_size_;
        Marshaler<SeedLimitOrder, true> r{{std::span<const char>(buffer, sz)}};
        
        EXPECT_FALSE(r.maxFloorQty.get().has_value());

        auto mpid = r.mpid.get();
        ASSERT_TRUE(mpid.has_value());
        EXPECT_EQ(*mpid, "BAML");
    }

    // Step 4: Re-add maxFloorQty with a different value
    EXPECT_TRUE((o.maxFloorQty = 200).has_value());
    EXPECT_EQ(o.state_.total_size_, sz_after_two);

    {
        size_t sz = o.state_.total_size_;
        Marshaler<SeedLimitOrder, true> r{{std::span<const char>(buffer, sz)}};

        auto maxf = r.maxFloorQty.get();
        ASSERT_TRUE(maxf.has_value());
        EXPECT_EQ(*maxf, 200u);

        auto mpid = r.mpid.get();
        ASSERT_TRUE(mpid.has_value());
        EXPECT_EQ(*mpid, "BAML");
    }
}
