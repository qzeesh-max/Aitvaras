# Copyright (c) 2026 Zeeshan Qazi <zeeshan@zeeshan.im>
#
# This file is part of Aitvaras.
#
# Aitvaras is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Aitvaras is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with Aitvaras.  If not, see <https://www.gnu.org/licenses/>.

content = """#ifndef AITVARAS_PROTOCOLS_HPP
#define AITVARAS_PROTOCOLS_HPP

#include <cstdint>
#include <string_view>
#include <span>
#include <aitvaras/marshaler.hpp>

namespace aitvaras {

// ---------------------------------------------------------
// RAKE TCP & UDP
// ---------------------------------------------------------
[[=packed{}]]
struct RakeLogonRequest {
    uint16_t length;
    uint8_t messageType; // '5'
    uint64_t session;
    [[=padded_string{' ', 8}]] std::string_view senderComp;
    [[=padded_string{' ', 8}]] std::string_view token;
    uint64_t nextSequenceNumber;
};

[[=packed{}]]
struct RakeLogonResponse {
    uint16_t length;
    uint8_t messageType; // '1'
    uint64_t session;
    uint64_t nextSequenceNumber;
    uint64_t highestKnownSequenceNumber;
    uint8_t responseCode;
    uint8_t numberStreamIDs;
    uint32_t instance;
};

[[=packed{}]]
struct RakeMemberHeartbeat {
    uint16_t length;
    uint8_t messageType; // '7'
};

[[=packed{}]]
struct RakeServerHeartbeat {
    uint16_t length;
    uint8_t messageType; // '3'
};

[[=packed{}]]
struct RakeTcpUnsequencedMessage {
    uint16_t length;
    uint8_t messageType; // '6'
    [[=length_precedes{^^length, -1}]] std::span<const uint8_t> payload;
};

[[=packed{}]]
struct RakeTcpSequencedMessage {
    uint16_t length;
    uint8_t messageType; // '2'
    uint8_t streamId;
    [[=length_precedes{^^length, -2}]] std::span<const uint8_t> payload;
};

[[=packed{}]]
struct RakeDebug {
    uint16_t length;
    uint8_t messageType; // '0'
    [[=length_precedes{^^length, -1}]] std::string_view payload;
};

[[=packed{}]]
struct RakeEndOfSession {
    uint16_t length;
    uint8_t messageType; // '4'
};

[[=packed{}]]
struct RakeUdpHeader {
    uint64_t session;
    uint64_t sequence;
    uint16_t messageCount;
    uint8_t type;
};

[[=packed{}]]
struct RakeUdpSequencedMessage {
    uint16_t length;
    uint8_t streamId;
    [[=length_precedes{^^length, -1}]] std::span<const uint8_t> payload;
};

// ---------------------------------------------------------
// SoupBinTCP 4.0
// ---------------------------------------------------------

[[=packed{}]]
struct SoupLoginRequest {
    [[=endian_swap{}]] uint16_t length;
    uint8_t packetType; // 'L'
    [[=padded_string{' ', 6}]] std::string_view username;
    [[=padded_string{' ', 10}]] std::string_view password;
    [[=padded_string{' ', 10}]] std::string_view requestedSession;
    [[=padded_string{' ', 20}]] std::string_view requestedSequenceNumber;
};

[[=packed{}]]
struct SoupLoginAccepted {
    [[=endian_swap{}]] uint16_t length;
    uint8_t packetType; // 'A'
    [[=padded_string{' ', 10}]] std::string_view session;
    [[=padded_string{' ', 20}]] std::string_view sequenceNumber;
};

[[=packed{}]]
struct SoupLoginRejected {
    [[=endian_swap{}]] uint16_t length;
    uint8_t packetType; // 'J'
    uint8_t rejectReasonCode;
};

[[=packed{}]]
struct SoupSequencedData {
    [[=endian_swap{}]] uint16_t length;
    uint8_t packetType; // 'S'
    [[=length_precedes{^^length, -1}]] std::span<const uint8_t> message;
};

[[=packed{}]]
struct SoupUnsequencedData {
    [[=endian_swap{}]] uint16_t length;
    uint8_t packetType; // 'U'
    [[=length_precedes{^^length, -1}]] std::span<const uint8_t> message;
};

[[=packed{}]]
struct SoupServerHeartbeat {
    [[=endian_swap{}]] uint16_t length;
    uint8_t packetType; // 'H'
};

[[=packed{}]]
struct SoupClientHeartbeat {
    [[=endian_swap{}]] uint16_t length;
    uint8_t packetType; // 'R'
};

[[=packed{}]]
struct SoupEndOfSession {
    [[=endian_swap{}]] uint16_t length;
    uint8_t packetType; // 'Z'
};

[[=packed{}]]
struct SoupLogoutRequest {
    [[=endian_swap{}]] uint16_t length;
    uint8_t packetType; // 'O'
};

[[=packed{}]]
struct SoupDebug {
    [[=endian_swap{}]] uint16_t length;
    uint8_t packetType; // '+'
    [[=length_precedes{^^length, -1}]] std::string_view text;
};

// ---------------------------------------------------------
// OUCH 5.0
// ---------------------------------------------------------

#define OUCH_APPENDAGES \\
    [[=tlv_appendage{1}]] std::span<const uint8_t> appendageSecondaryOrdRefNum; \\
    [[=tlv_appendage{2}]] std::span<const uint8_t> appendageFirm; \\
    [[=tlv_appendage{3}]] std::span<const uint8_t> appendageMinQty; \\
    [[=tlv_appendage{4}]] std::span<const uint8_t> appendageCustomerType; \\
    [[=tlv_appendage{5}]] std::span<const uint8_t> appendageMaxFloor; \\
    [[=tlv_appendage{6}]] std::span<const uint8_t> appendagePriceType; \\
    [[=tlv_appendage{7}]] std::span<const uint8_t> appendagePegOffset; \\
    [[=tlv_appendage{9}]] std::span<const uint8_t> appendageDiscretionPrice; \\
    [[=tlv_appendage{10}]] std::span<const uint8_t> appendageDiscretionPriceType; \\
    [[=tlv_appendage{11}]] std::span<const uint8_t> appendageDiscretionPegOffset; \\
    [[=tlv_appendage{12}]] std::span<const uint8_t> appendagePostOnly; \\
    [[=tlv_appendage{13}]] std::span<const uint8_t> appendageRandomReserves; \\
    [[=tlv_appendage{14}]] std::span<const uint8_t> appendageRoute; \\
    [[=tlv_appendage{15}]] std::span<const uint8_t> appendageExpireTime; \\
    [[=tlv_appendage{16}]] std::span<const uint8_t> appendageTradeNow; \\
    [[=tlv_appendage{17}]] std::span<const uint8_t> appendageHandleInst; \\
    [[=tlv_appendage{18}]] std::span<const uint8_t> appendageBboWeightIndicator;

[[=packed{}]]
struct OuchEnterOrder {
    uint8_t type; // 'O'
    uint32_t userRefNum;
    uint8_t side;
    uint32_t quantity;
    [[=padded_string{' ', 8}]] std::string_view symbol;
    [[=fixed_point{4}]] uint64_t price;
    uint8_t timeInForce;
    uint8_t display;
    uint8_t capacity;
    uint8_t interMarketSweepEligibility;
    uint8_t crossType;
    [[=padded_string{' ', 14}]] std::string_view clOrdID;
    
    [[=tlv_region_length{}]]
    uint16_t appendageLength;
    
    [[=length_precedes{^^appendageLength, 0}]] 
    std::span<const uint8_t> optionalAppendageBlob;

    OUCH_APPENDAGES
};

[[=packed{}]]
struct OuchReplaceOrder {
    uint8_t type; // 'U'
    uint32_t origUserRefNum;
    uint32_t userRefNum;
    uint32_t quantity;
    [[=fixed_point{4}]] uint64_t price;
    uint8_t timeInForce;
    uint8_t display;
    uint8_t interMarketSweepEligibility;
    [[=padded_string{' ', 14}]] std::string_view clOrdID;
    
    [[=tlv_region_length{}]]
    uint16_t appendageLength;
    
    [[=length_precedes{^^appendageLength, 0}]] 
    std::span<const uint8_t> optionalAppendageBlob;

    OUCH_APPENDAGES
};

[[=packed{}]]
struct OuchCancelOrder {
    uint8_t type; // 'X'
    uint32_t userRefNum;
    uint32_t quantity;
    
    [[=tlv_region_length{}]]
    uint16_t appendageLength;
    
    [[=length_precedes{^^appendageLength, 0}]] 
    std::span<const uint8_t> optionalAppendageBlob;

    OUCH_APPENDAGES
};

[[=packed{}]]
struct OuchModifyOrder {
    uint8_t type; // 'M'
    uint32_t userRefNum;
    uint8_t side;
    uint32_t quantity;
    
    [[=tlv_region_length{}]]
    uint16_t appendageLength;
    
    [[=length_precedes{^^appendageLength, 0}]] 
    std::span<const uint8_t> optionalAppendageBlob;

    OUCH_APPENDAGES
};

[[=packed{}]]
struct OuchMassCancel {
    uint8_t type; // 'C'
    uint32_t number;
    [[=padded_string{' ', 4}]] std::string_view firm;
    [[=padded_string{' ', 8}]] std::string_view symbol;
    
    [[=tlv_region_length{}]]
    uint16_t appendageLength;
    
    [[=length_precedes{^^appendageLength, 0}]] 
    std::span<const uint8_t> optionalAppendageBlob;

    OUCH_APPENDAGES
};

[[=packed{}]]
struct OuchSystemEvent {
    uint8_t type; // 'S'
    uint64_t timestamp;
    uint8_t eventCode;
};

[[=packed{}]]
struct OuchOrderAccepted {
    uint8_t type; // 'A'
    uint64_t timestamp;
    uint32_t userRefNum;
    uint8_t side;
    uint32_t quantity;
    [[=padded_string{' ', 8}]] std::string_view symbol;
    [[=fixed_point{4}]] uint64_t price;
    uint8_t timeInForce;
    uint8_t display;
    uint64_t orderReferenceNumber;
    uint8_t capacity;
    uint8_t interMarketSweepEligibility;
    uint8_t crossType;
    uint8_t orderState;
    [[=padded_string{' ', 14}]] std::string_view clOrdID;
    
    [[=tlv_region_length{}]]
    uint16_t appendageLength;
    
    [[=length_precedes{^^appendageLength, 0}]] 
    std::span<const uint8_t> optionalAppendageBlob;

    OUCH_APPENDAGES
};

[[=packed{}]]
struct OuchOrderReplaced {
    uint8_t type; // 'U'
    uint64_t timestamp;
    uint32_t origUserRefNum;
    uint32_t userRefNum;
    uint8_t side;
    uint32_t quantity;
    [[=padded_string{' ', 8}]] std::string_view symbol;
    [[=fixed_point{4}]] uint64_t price;
    uint8_t timeInForce;
    uint8_t display;
    uint64_t orderReferenceNumber;
    uint8_t capacity;
    uint8_t interMarketSweepEligibility;
    uint8_t crossType;
    uint8_t orderState;
    [[=padded_string{' ', 14}]] std::string_view clOrdID;
    
    [[=tlv_region_length{}]]
    uint16_t appendageLength;
    
    [[=length_precedes{^^appendageLength, 0}]] 
    std::span<const uint8_t> optionalAppendageBlob;

    OUCH_APPENDAGES
};

[[=packed{}]]
struct OuchOrderCanceled {
    uint8_t type; // 'C'
    uint64_t timestamp;
    uint32_t userRefNum;
    uint32_t quantity;
};

[[=packed{}]]
struct OuchAiqCanceled {
    uint8_t type; // 'D'
    uint64_t timestamp;
    uint32_t userRefNum;
    uint32_t decrementShares;
    uint8_t reason;
    uint32_t quantityPreventedFromTrading;
    [[=fixed_point{4}]] uint64_t executionPrice;
};

[[=packed{}]]
struct OuchOrderExecuted {
    uint8_t type; // 'E'
    uint64_t timestamp;
    uint32_t userRefNum;
    uint32_t quantity;
    [[=fixed_point{4}]] uint64_t price;
    uint8_t liquidityFlag;
    uint64_t matchNumber;
};

[[=packed{}]]
struct OuchBrokenTrade {
    uint8_t type; // 'B'
    uint64_t timestamp;
    uint32_t userRefNum;
    uint64_t matchNumber;
    uint8_t reason;
    [[=padded_string{' ', 14}]] std::string_view clOrdID;
    
    [[=tlv_region_length{}]]
    uint16_t appendageLength;
    
    [[=length_precedes{^^appendageLength, 0}]] 
    std::span<const uint8_t> optionalAppendageBlob;

    OUCH_APPENDAGES
};

[[=packed{}]]
struct OuchRejected {
    uint8_t type; // 'J'
    uint64_t timestamp;
    uint32_t userRefNum;
    uint16_t reason;
    [[=padded_string{' ', 14}]] std::string_view clOrdID;
    
    [[=tlv_region_length{}]]
    uint16_t appendageLength;
    
    [[=length_precedes{^^appendageLength, 0}]] 
    std::span<const uint8_t> optionalAppendageBlob;

    OUCH_APPENDAGES
};

[[=packed{}]]
struct OuchCancelPending {
    uint8_t type; // 'P'
    uint64_t timestamp;
    uint32_t userRefNum;
    
    [[=tlv_region_length{}]]
    uint16_t appendageLength;
    
    [[=length_precedes{^^appendageLength, 0}]] 
    std::span<const uint8_t> optionalAppendageBlob;

    OUCH_APPENDAGES
};

[[=packed{}]]
struct OuchCancelReject {
    uint8_t type; // 'I'
    uint64_t timestamp;
    uint32_t userRefNum;
    
    [[=tlv_region_length{}]]
    uint16_t appendageLength;
    
    [[=length_precedes{^^appendageLength, 0}]] 
    std::span<const uint8_t> optionalAppendageBlob;

    OUCH_APPENDAGES
};

[[=packed{}]]
struct OuchOrderPriorityUpdate {
    uint8_t type; // 'T'
    uint64_t timestamp;
    uint32_t userRefNum;
    [[=fixed_point{4}]] uint64_t price;
    uint8_t display;
    uint64_t orderReferenceNumber;
    
    [[=tlv_region_length{}]]
    uint16_t appendageLength;
    
    [[=length_precedes{^^appendageLength, 0}]] 
    std::span<const uint8_t> optionalAppendageBlob;

    OUCH_APPENDAGES
};

[[=packed{}]]
struct OuchOrderModified {
    uint8_t type; // 'M'
    uint64_t timestamp;
    uint32_t userRefNum;
    uint8_t side;
    uint32_t quantity;
    
    [[=tlv_region_length{}]]
    uint16_t appendageLength;
    
    [[=length_precedes{^^appendageLength, 0}]] 
    std::span<const uint8_t> optionalAppendageBlob;

    OUCH_APPENDAGES
};


// ---------------------------------------------------------
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

#endif // AITVARAS_PROTOCOLS_HPP
"""
with open("test/protocols.hpp", "w") as f:
    f.write(content)
