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

#include <gtest/gtest.h>
#include <aitvaras/marshaler.hpp>
#include "protocol_rake.hpp"

using namespace aitvaras;

TEST(RakeGeneratedTest, RakeLogonResponse) {
    char buffer[1024] = {0};
    Marshaler<RakeLogonResponse> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.length = 100;
    msg.messageType = 'X';
    msg.session = 3;
    msg.nextSequenceNumber = 4;
    msg.highestKnownSequenceNumber = 5;
    msg.responseCode = 6;
    msg.numberStreamIDs = 7;
    msg.instance = 8;
}

TEST(RakeGeneratedTest, RakeMemberHeartbeat) {
    char buffer[1024] = {0};
    Marshaler<RakeMemberHeartbeat> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.length = 100;
    msg.messageType = 'X';
}

TEST(RakeGeneratedTest, RakeServerHeartbeat) {
    char buffer[1024] = {0};
    Marshaler<RakeServerHeartbeat> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.length = 100;
    msg.messageType = 'X';
}

TEST(RakeGeneratedTest, RakeEndOfSession) {
    char buffer[1024] = {0};
    Marshaler<RakeEndOfSession> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.length = 100;
    msg.messageType = 'X';
}

TEST(RakeGeneratedTest, RakeUdpHeader) {
    char buffer[1024] = {0};
    Marshaler<RakeUdpHeader> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.session = 1;
    msg.sequence = 2;
    msg.messageCount = 3;
    msg.type = 'X';
}

