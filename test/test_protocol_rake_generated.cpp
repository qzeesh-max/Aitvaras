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

