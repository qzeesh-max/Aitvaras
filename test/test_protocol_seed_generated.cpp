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
#include "protocol_seed.hpp"

using namespace aitvaras;

TEST(SeedGeneratedTest, SeedCancelOrder) {
    char buffer[1024] = {0};
    Marshaler<SeedCancelOrder> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.messageType = 'X';
    msg.clOrdId = 3;
    msg.origClOrdId = 4;
}

TEST(SeedGeneratedTest, SeedModifyOrder) {
    char buffer[1024] = {0};
    Marshaler<SeedModifyOrder> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.messageType = 'X';
    msg.clOrdId = 3;
    msg.origClOrdId = 4;
    msg.modifyOrderBitFields = 5;
    msg.orderQty = 6;
}

TEST(SeedGeneratedTest, SeedLimitOrderRejected) {
    char buffer[1024] = {0};
    Marshaler<SeedLimitOrderRejected> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.messageType = 'X';
    msg.clOrdId = 3;
    msg.timestamp = 4;
    msg.rejectReason = 5;
}

TEST(SeedGeneratedTest, SeedMarketOrderAccepted) {
    char buffer[1024] = {0};
    Marshaler<SeedMarketOrderAccepted> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.messageType = 'X';
    msg.clOrdId = 3;
    msg.orderId = 4;
    msg.timestamp = 5;
    msg.orderQty = 6;
    msg.symbolId = 7;
}

TEST(SeedGeneratedTest, SeedMarketOrderRejected) {
    char buffer[1024] = {0};
    Marshaler<SeedMarketOrderRejected> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.messageType = 'X';
    msg.clOrdId = 3;
    msg.timestamp = 4;
    msg.rejectReason = 5;
}

TEST(SeedGeneratedTest, SeedOrderCanceled) {
    char buffer[1024] = {0};
    Marshaler<SeedOrderCanceled> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.messageType = 'X';
    msg.clOrdId = 3;
    msg.orderId = 4;
    msg.timestamp = 5;
    msg.canceledQty = 6;
    msg.cancelReason = 7;
}

TEST(SeedGeneratedTest, SeedCancelRejected) {
    char buffer[1024] = {0};
    Marshaler<SeedCancelRejected> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.messageType = 'X';
    msg.clOrdId = 3;
    msg.timestamp = 4;
    msg.rejectReason = 5;
}

TEST(SeedGeneratedTest, SeedOrderModified) {
    char buffer[1024] = {0};
    Marshaler<SeedOrderModified> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.messageType = 'X';
    msg.clOrdId = 3;
    msg.orderId = 4;
    msg.timestamp = 5;
    msg.orderQty = 6;
    msg.leavesQty = 7;
}

TEST(SeedGeneratedTest, SeedModifyRejected) {
    char buffer[1024] = {0};
    Marshaler<SeedModifyRejected> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.messageType = 'X';
    msg.clOrdId = 3;
    msg.timestamp = 4;
    msg.rejectReason = 5;
}

TEST(SeedGeneratedTest, SeedReplaceRejected) {
    char buffer[1024] = {0};
    Marshaler<SeedReplaceRejected> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.messageType = 'X';
    msg.clOrdId = 3;
    msg.timestamp = 4;
    msg.rejectReason = 5;
}

TEST(SeedGeneratedTest, SeedMassCancelAccepted) {
    char buffer[1024] = {0};
    Marshaler<SeedMassCancelAccepted> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.messageType = 'X';
    msg.clOrdId = 3;
    msg.timestamp = 4;
}

TEST(SeedGeneratedTest, SeedMassCancelRejected) {
    char buffer[1024] = {0};
    Marshaler<SeedMassCancelRejected> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.messageType = 'X';
    msg.clOrdId = 3;
    msg.timestamp = 4;
    msg.rejectReason = 5;
}

TEST(SeedGeneratedTest, SeedMassCancelResult) {
    char buffer[1024] = {0};
    Marshaler<SeedMassCancelResult> msg{{std::span<char>(buffer, sizeof(buffer))}};

    msg.messageType = 'X';
    msg.clOrdId = 3;
    msg.timestamp = 4;
    msg.canceledCount = 5;
}

