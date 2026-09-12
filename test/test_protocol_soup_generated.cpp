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

TEST(SoupGeneratedTest, SoupLoginRejected) {
    char buffer[1024] = {0};
    Marshaler<SoupLoginRejected> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.length = 100;
    msg.packetType = 'X';
    msg.rejectReasonCode = 3;
}

TEST(SoupGeneratedTest, SoupServerHeartbeat) {
    char buffer[1024] = {0};
    Marshaler<SoupServerHeartbeat> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.length = 100;
    msg.packetType = 'X';
}

TEST(SoupGeneratedTest, SoupClientHeartbeat) {
    char buffer[1024] = {0};
    Marshaler<SoupClientHeartbeat> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.length = 100;
    msg.packetType = 'X';
}

TEST(SoupGeneratedTest, SoupEndOfSession) {
    char buffer[1024] = {0};
    Marshaler<SoupEndOfSession> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.length = 100;
    msg.packetType = 'X';
}

TEST(SoupGeneratedTest, SoupLogoutRequest) {
    char buffer[1024] = {0};
    Marshaler<SoupLogoutRequest> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.length = 100;
    msg.packetType = 'X';
}

