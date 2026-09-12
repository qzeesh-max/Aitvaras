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

