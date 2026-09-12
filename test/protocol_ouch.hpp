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
// OUCH 5.0
// ---------------------------------------------------------

#define OUCH_APPENDAGES \
    [[=tlv_appendage{1}]] uint64_t appendageSecondaryOrdRefNum; \
    [[=tlv_appendage{2}]] [[=padded_string{' ', 4}]] std::string_view appendageFirm; \
    [[=tlv_appendage{3}]] uint32_t appendageMinQty; \
    [[=tlv_appendage{4}]] uint8_t appendageCustomerType; \
    [[=tlv_appendage{5}]] uint32_t appendageMaxFloor; \
    [[=tlv_appendage{6}]] uint8_t appendagePriceType; \
    [[=tlv_appendage{7}]] int32_t appendagePegOffset; \
    [[=tlv_appendage{9}]] [[=fixed_point{4}]] uint64_t appendageDiscretionPrice; \
    [[=tlv_appendage{10}]] uint8_t appendageDiscretionPriceType; \
    [[=tlv_appendage{11}]] int32_t appendageDiscretionPegOffset; \
    [[=tlv_appendage{12}]] uint8_t appendagePostOnly; \
    [[=tlv_appendage{13}]] uint32_t appendageRandomReserves; \
    [[=tlv_appendage{14}]] [[=padded_string{' ', 4}]] std::string_view appendageRoute; \
    [[=tlv_appendage{15}]] uint32_t appendageExpireTime; \
    [[=tlv_appendage{16}]] uint8_t appendageTradeNow; \
    [[=tlv_appendage{17}]] uint8_t appendageHandleInst; \
    [[=tlv_appendage{18}]] uint8_t appendageBboWeightIndicator;

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
} // namespace aitvaras
