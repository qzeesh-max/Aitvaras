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
#include "protocol_ouch.hpp"

using namespace aitvaras;

TEST(OuchGeneratedTest, OuchSystemEvent) {
    char buffer[1024] = {0};
    Marshaler<OuchSystemEvent> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.type = 'X';
    msg.timestamp = 2;
    msg.eventCode = 3;
}

TEST(OuchGeneratedTest, OuchOrderCanceled) {
    char buffer[1024] = {0};
    Marshaler<OuchOrderCanceled> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.type = 'X';
    msg.timestamp = 2;
    msg.userRefNum = 3;
    msg.quantity = 4;
}

