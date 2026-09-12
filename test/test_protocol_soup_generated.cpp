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

