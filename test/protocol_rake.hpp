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
// RAKE TCP & UDP
// ---------------------------------------------------------
[[=packed{}]] [[=big_endian{}]]
struct RakeLogonRequest {
    uint16_t length;
    uint8_t messageType; // '5'
    uint64_t session;
    [[=padded_string{' ', 8}]] std::string_view senderComp;
    [[=padded_string{' ', 8}]] std::string_view token;
    uint64_t nextSequenceNumber;
};

[[=packed{}]] [[=big_endian{}]]
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

[[=packed{}]] [[=big_endian{}]]
struct RakeMemberHeartbeat {
    uint16_t length;
    uint8_t messageType; // '7'
};

[[=packed{}]] [[=big_endian{}]]
struct RakeServerHeartbeat {
    uint16_t length;
    uint8_t messageType; // '3'
};

[[=packed{}]] [[=big_endian{}]]
struct RakeTcpUnsequencedMessage {
    uint16_t length;
    uint8_t messageType; // '6'
    [[=length_precedes{^^length, -1}]] std::span<const uint8_t> payload;
};

[[=packed{}]] [[=big_endian{}]]
struct RakeTcpSequencedMessage {
    uint16_t length;
    uint8_t messageType; // '2'
    uint8_t streamId;
    [[=length_precedes{^^length, -2}]] std::span<const uint8_t> payload;
};

[[=packed{}]] [[=big_endian{}]]
struct RakeDebug {
    uint16_t length;
    uint8_t messageType; // '0'
    [[=length_precedes{^^length, -1}]] std::string_view payload;
};

[[=packed{}]] [[=big_endian{}]]
struct RakeEndOfSession {
    uint16_t length;
    uint8_t messageType; // '4'
};

[[=packed{}]] [[=big_endian{}]]
struct RakeUdpHeader {
    uint64_t session;
    uint64_t sequence;
    uint16_t messageCount;
    uint8_t type;
};

[[=packed{}]] [[=big_endian{}]]
struct RakeUdpSequencedMessage {
    uint16_t length;
    uint8_t streamId;
    [[=length_precedes{^^length, -1}]] std::span<const uint8_t> payload;
};

// ---------------------------------------------------------
} // namespace aitvaras
