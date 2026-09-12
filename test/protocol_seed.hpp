/*
 * Copyright (c) 2026 Zeeshan Qazi <zeeshan@zeeshan.im>
 *
 * This file is part of Aitvaras.
 *
 * Aitvaras is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Aitvaras is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Aitvaras.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#include <cstdint>
#include <string_view>
#include <span>
#include <aitvaras/annotations.hpp>

namespace aitvaras {
// Texas SEED
// ---------------------------------------------------------
enum class SeedSide : uint8_t { Buy = 0, LongSell = 1, ShortSell = 2, ShortExempt = 3 };
enum class SeedTimeInForce : uint8_t { Sys = 1, Ioc = 2, Gtt = 3, Day = 4, Rho = 5, AtTheOpen = 6, AtTheClose = 7 };

[[=packed{}]]
struct SeedLimitOrder {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint32_t orderQty;
    uint32_t limitOrderBitFields;
    uint16_t symbolId;
    [[=fixed_point{8}]] uint64_t price;
    
    [[=non_fixed_bitmap{^^SeedLimitOrder::presenceBits, 0}]] uint8_t selfMatchScope;
    [[=non_fixed_bitmap{^^SeedLimitOrder::presenceBits, 1}]] uint8_t selfMatchInstruction;
    [[=non_fixed_bitmap{^^SeedLimitOrder::presenceBits, 2}]] uint8_t priceSlideInstruction;
    [[=non_fixed_bitmap{^^SeedLimitOrder::presenceBits, 3}]] uint32_t minQty;
    [[=non_fixed_bitmap{^^SeedLimitOrder::presenceBits, 4}]] uint32_t maxFloorQty;
    [[=non_fixed_bitmap{^^SeedLimitOrder::presenceBits, 5}]] uint32_t maxReplenishQtyRange;
    [[=non_fixed_bitmap{^^SeedLimitOrder::presenceBits, 6}]] uint64_t maxReplenishTimeRange;
    [[=non_fixed_bitmap{^^SeedLimitOrder::presenceBits, 7}]] uint16_t referencePriceTarget;
    [[=non_fixed_bitmap{^^SeedLimitOrder::presenceBits, 8}]] uint64_t expireTime;
    [[=non_fixed_bitmap{^^SeedLimitOrder::presenceBits, 9}]] uint64_t userData;
    [[=non_fixed_bitmap{^^SeedLimitOrder::presenceBits, 10}]] [[=padded_string{' ', 4}]] std::string_view mpid;
    [[=non_fixed_bitmap{^^SeedLimitOrder::presenceBits, 11}]] [[=padded_string{' ', 2}]] std::string_view memberGroup;
    [[=non_fixed_bitmap{^^SeedLimitOrder::presenceBits, 12}]] [[=padded_string{' ', 4}]] std::string_view locateBroker;
};

[[=packed{}]]
struct SeedMarketOrder {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint32_t orderQty;
    uint32_t marketOrderBitFields;
    uint16_t symbolId;
    
    [[=non_fixed_bitmap{^^SeedMarketOrder::presenceBits, 0}]] uint8_t selfMatchScope;
    [[=non_fixed_bitmap{^^SeedMarketOrder::presenceBits, 1}]] uint8_t selfMatchInstruction;
    [[=non_fixed_bitmap{^^SeedMarketOrder::presenceBits, 2}]] uint32_t minQty;
    [[=non_fixed_bitmap{^^SeedMarketOrder::presenceBits, 3}]] uint64_t expireTime;
    [[=non_fixed_bitmap{^^SeedMarketOrder::presenceBits, 4}]] uint64_t userData;
    [[=non_fixed_bitmap{^^SeedMarketOrder::presenceBits, 5}]] [[=padded_string{' ', 4}]] std::string_view mpid;
    [[=non_fixed_bitmap{^^SeedMarketOrder::presenceBits, 6}]] [[=padded_string{' ', 2}]] std::string_view memberGroup;
    [[=non_fixed_bitmap{^^SeedMarketOrder::presenceBits, 7}]] [[=padded_string{' ', 4}]] std::string_view locateBroker;
};

[[=packed{}]]
struct SeedCancelOrder {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t origClOrdId;
};

[[=packed{}]]
struct SeedModifyOrder {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t origClOrdId;
    uint32_t modifyOrderBitFields;
    uint32_t orderQty;
};

[[=packed{}]]
struct SeedReplaceOrder {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t origClOrdId;
    uint32_t orderQty;
    uint32_t replaceOrderBitFields;
    [[=fixed_point{8}]] uint64_t price;
    
    [[=non_fixed_bitmap{^^SeedReplaceOrder::presenceBits, 0}]] uint32_t minQty;
    [[=non_fixed_bitmap{^^SeedReplaceOrder::presenceBits, 1}]] uint32_t maxFloorQty;
    [[=non_fixed_bitmap{^^SeedReplaceOrder::presenceBits, 2}]] uint32_t maxReplenishQtyRange;
    [[=non_fixed_bitmap{^^SeedReplaceOrder::presenceBits, 3}]] uint64_t maxReplenishTimeRange;
    [[=non_fixed_bitmap{^^SeedReplaceOrder::presenceBits, 4}]] uint16_t referencePriceTarget;
};

[[=packed{}]]
struct SeedMassCancel {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint8_t massCancelScope;
    uint16_t symbolId;
    [[=padded_string{' ', 4}]] std::string_view mpid;
    [[=padded_string{' ', 2}]] std::string_view memberGroup;
};

// ---------------------------------------------------------
// SEED Outbound
// ---------------------------------------------------------
[[=packed{}]]
struct SeedLimitOrderAccepted {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t orderId;
    uint64_t timestamp;
    uint32_t orderQty;
    uint16_t symbolId;
    [[=fixed_point{8}]] uint64_t price;
};

[[=packed{}]]
struct SeedLimitOrderRejected {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t timestamp;
    uint16_t rejectReason;
};

[[=packed{}]]
struct SeedMarketOrderAccepted {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t orderId;
    uint64_t timestamp;
    uint32_t orderQty;
    uint16_t symbolId;
};

[[=packed{}]]
struct SeedMarketOrderRejected {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t timestamp;
    uint16_t rejectReason;
};

[[=packed{}]]
struct SeedOrderCanceled {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t orderId;
    uint64_t timestamp;
    uint32_t canceledQty;
    uint8_t cancelReason;
};

[[=packed{}]]
struct SeedCancelRejected {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t timestamp;
    uint16_t rejectReason;
};

[[=packed{}]]
struct SeedOrderModified {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t orderId;
    uint64_t timestamp;
    uint32_t orderQty;
    uint32_t leavesQty;
};

[[=packed{}]]
struct SeedModifyRejected {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t timestamp;
    uint16_t rejectReason;
};

[[=packed{}]]
struct SeedOrderReplaced {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t orderId;
    uint64_t timestamp;
    uint32_t orderQty;
    [[=fixed_point{8}]] uint64_t price;
    uint32_t leavesQty;
};

[[=packed{}]]
struct SeedReplaceRejected {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t timestamp;
    uint16_t rejectReason;
};

[[=packed{}]]
struct SeedOrderExecuted {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t orderId;
    uint64_t timestamp;
    uint64_t execId;
    uint32_t lastQty;
    [[=fixed_point{8}]] uint64_t lastPx;
    uint32_t leavesQty;
    uint8_t liquidityIndicator;
};

[[=packed{}]]
struct SeedOrderRestated {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t orderId;
    uint64_t timestamp;
    uint8_t restatementReason;
    
    [[=non_fixed_bitmap{^^SeedOrderRestated::presenceBits, 0}]] uint32_t leavesQty;
    [[=non_fixed_bitmap{^^SeedOrderRestated::presenceBits, 1}]] uint32_t displayQty;
    [[=non_fixed_bitmap{^^SeedOrderRestated::presenceBits, 2}]] [[=fixed_point{8}]] uint64_t displayPrice;
    [[=non_fixed_bitmap{^^SeedOrderRestated::presenceBits, 3}]] [[=fixed_point{8}]] uint64_t pegPrice;
};

[[=packed{}]]
struct SeedSelfMatchPrevented {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t orderId;
    uint64_t timestamp;
    uint64_t matchId;
    uint32_t canceledQty;
    uint32_t leavesQty;
    uint32_t preventedQty;
    [[=fixed_point{8}]] uint64_t preventedPx;
};

[[=packed{}]]
struct SeedMassCancelAccepted {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t timestamp;
};

[[=packed{}]]
struct SeedMassCancelRejected {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t timestamp;
    uint16_t rejectReason;
};

[[=packed{}]]
struct SeedMassCancelResult {
    uint8_t messageType;
    uint32_t presenceBits;
    uint64_t clOrdId;
    uint64_t timestamp;
    uint32_t canceledCount;
};
} // namespace aitvaras
