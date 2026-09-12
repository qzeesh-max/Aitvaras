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
