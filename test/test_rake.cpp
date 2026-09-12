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
#include "protocol_rake.hpp"

using namespace aitvaras;


// RAKE Frame Tests
// ============================================================

TEST(RakeTest, LogonRequestRoundTrip) {
    char buffer[256] = {0};
    {
        Marshaler<RakeLogonRequest> req{{std::span<char>(buffer, sizeof(buffer))}};
        req.messageType = '5';
        req.session = 0xDEADBEEFCAFEBABEULL;
        req.senderComp = "MYSEND";
        req.token = "MYTOKEN";
        req.nextSequenceNumber = 42ULL;
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
        msg.messageType = '2';
        msg.streamId = 3;
        msg.payload = std::span<const uint8_t>(payload, sizeof(payload));
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
        msg.messageType = '0';
        msg.payload = "Hello debug!";
        (void)(msg.length = static_cast<uint16_t>(msg.state_.total_size_ - 2));

        EXPECT_EQ(msg.state_.total_size_, 15u);  // 2+1+12
    }
    Marshaler<RakeDebug, true> reader{{std::span<const char>(buffer, 15)}};
    EXPECT_EQ(reader.messageType.get(), '0');
    EXPECT_EQ(reader.payload.get(), "Hello debug!");
}

// ============================================================
