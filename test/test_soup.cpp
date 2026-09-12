/*
 * Copyright (c) 2026 Zeeshan Qazi
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

#include <gtest/gtest.h>
#include <aitvaras/marshaler.hpp>
#include "protocol_soup.hpp"

using namespace aitvaras;


// SOUP Frame Tests
// ============================================================

TEST(SoupTest, LoginRequestRoundTrip) {
    char buffer[256] = {0};
    {
        Marshaler<SoupLoginRequest> req{{std::span<char>(buffer, sizeof(buffer))}};
        req.packetType = 'L';
        req.username = "USER01";
        req.password = "PASS01";
        req.requestedSession = "SESSION1";
        req.requestedSequenceNumber = "1";
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
        msg.packetType = 'S';
        msg.message = std::span<const uint8_t>(ouch_bytes, sizeof(ouch_bytes));
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
