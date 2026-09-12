#pragma once
#include <cstdint>
#include <string_view>
#include <span>
#include <aitvaras/annotations.hpp>

namespace aitvaras {
// SoupBinTCP 4.0
// ---------------------------------------------------------

[[=packed{}]]
[[=big_endian{}]]
struct SoupLoginRequest {
    uint16_t length;
    uint8_t packetType; // 'L'
    [[=padded_string{' ', 6}]] std::string_view username;
    [[=padded_string{' ', 10}]] std::string_view password;
    [[=padded_string{' ', 10}]] std::string_view requestedSession;
    [[=padded_string{' ', 20}]] std::string_view requestedSequenceNumber;
};

[[=packed{}]]
[[=big_endian{}]]
struct SoupLoginAccepted {
    uint16_t length;
    uint8_t packetType; // 'A'
    [[=padded_string{' ', 10}]] std::string_view session;
    [[=padded_string{' ', 20}]] std::string_view sequenceNumber;
};

[[=packed{}]]
[[=big_endian{}]]
struct SoupLoginRejected {
    uint16_t length;
    uint8_t packetType; // 'J'
    uint8_t rejectReasonCode;
};

[[=packed{}]]
[[=big_endian{}]]
struct SoupSequencedData {
    uint16_t length;
    uint8_t packetType; // 'S'
    [[=length_precedes{^^length, -1}]] std::span<const uint8_t> message;
};

[[=packed{}]]
[[=big_endian{}]]
struct SoupUnsequencedData {
    uint16_t length;
    uint8_t packetType; // 'U'
    [[=length_precedes{^^length, -1}]] std::span<const uint8_t> message;
};

[[=packed{}]]
[[=big_endian{}]]
struct SoupServerHeartbeat {
    uint16_t length;
    uint8_t packetType; // 'H'
};

[[=packed{}]]
[[=big_endian{}]]
struct SoupClientHeartbeat {
    uint16_t length;
    uint8_t packetType; // 'R'
};

[[=packed{}]]
[[=big_endian{}]]
struct SoupEndOfSession {
    uint16_t length;
    uint8_t packetType; // 'Z'
};

[[=packed{}]]
[[=big_endian{}]]
struct SoupLogoutRequest {
    uint16_t length;
    uint8_t packetType; // 'O'
};

[[=packed{}]]
[[=big_endian{}]]
struct SoupDebug {
    uint16_t length;
    uint8_t packetType; // '+'
    [[=length_precedes{^^length, -1}]] std::string_view text;
};

// ---------------------------------------------------------
} // namespace aitvaras
