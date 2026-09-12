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

using namespace aitvaras;

enum class SideEnum : uint8_t { Buy = 1, Sell = 2 };
enum class OrderType : uint8_t { Limit = 1, Market = 2 };

constexpr uint64_t getnanosecondtimestamp() {
    return 123456789; // Dummy value for testing
}

[[=aitvaras::packed{}]]
struct Header {
    uint8_t packetType;
    uint32_t packetSize;
    uint8_t msgType;
};

constexpr uint8_t NewOrderType = 1;
constexpr uint8_t Unsequened = 0;

[[=aitvaras::packed{}]]
struct NewOrder {
    Header header {Unsequened, 0, NewOrderType};
    
    [[=aitvaras::default_invoke{^^getnanosecondtimestamp}]]
    uint64_t timeStamp;
    
    [[=aitvaras::padded_string{' ', 8}]]
    std::string_view symbol;
    
    [[=aitvaras::enumeration{^^SideEnum}]]
    uint8_t side;
    
    uint32_t orderQty;
    
    [[=aitvaras::fixed_point{6}]]
    uint64_t price;
    
    uint64_t orderToken;
    
    uint32_t optionalFields;
    
    uint8_t memoSize;
    
    [[=aitvaras::non_fixed_bitmap{^^optionalFields, 0}]]
    [[=aitvaras::length_precedes{^^memoSize, 0}]] // Just a test mapping
    std::string_view memo;
    
    [[=aitvaras::non_fixed_bitmap{^^optionalFields, 1}]]
    [[=aitvaras::enumeration{^^OrderType}]]
    uint8_t orderType;
};

TEST(MarshalerTest, ReflectionSizes) {
    static constexpr auto mems = std::define_static_array(
        std::meta::members_of(^^NewOrder, std::meta::access_context::current()));
    size_t offset = 0;
    template for (constexpr auto m : mems) {
        if constexpr (std::meta::is_nonstatic_data_member(m)) {
            if constexpr (!detail::is_variable_field<m>()) {
                size_t s = detail::get_fixed_field_size<m>();
                // std::cout << std::meta::identifier_of(m) << " : " << s << "\n";
            }
        }
    }
    
    char buffer[1024] = {0};
    Marshaler<NewOrder, false> proxy{{std::span<char>(buffer, sizeof(buffer))}};
    
    static_assert(aitvaras::detail::has_annotation<aitvaras::length_precedes, ^^NewOrder::memo>(), "memo should have length_precedes");
    
    // Set a fixed size field
    proxy.symbol = "AAPL";
    
    // Check that it's written (symbol is a padded_string of length 8)
    std::string_view sym = proxy.symbol;
    EXPECT_EQ(sym, "AAPL");
    
    // Set an arithmetic field
    proxy.price = 150.5; // price is fixed_point<6>
    double price_opt = proxy.price.get();
    EXPECT_DOUBLE_EQ(price_opt, 150.5);
    
    // Set a variable length string
    auto res3 = (proxy.memo = "Some order memo");
    EXPECT_TRUE(res3.has_value());
    
    // Read it back
    std::optional<std::string_view> memo_val = proxy.memo;
    EXPECT_TRUE(memo_val.has_value());
    EXPECT_EQ(*memo_val, "Some order memo");
    
    // Set a non_fixed_bitmap enum field
    auto res4 = (proxy.orderType = OrderType::Market);
    EXPECT_TRUE(res4.has_value());
    
    // Check that the bitmask was updated
    uint32_t opt_fields = proxy.optionalFields.get();
    EXPECT_EQ(opt_fields, 3); // bits 0 and 1 should be set
    
    // Check that we can read it back
    auto order_type_val = proxy.orderType.get();
    ASSERT_TRUE(order_type_val.has_value());
    EXPECT_EQ(*order_type_val, 2);
    
    // Check total size updated
    EXPECT_GT(proxy.state_.total_size_, 30);
}

struct TestMessage {
    uint8_t msgType;
    uint32_t length;
    uint32_t optionalFields;
    
    [[=aitvaras::non_fixed_bitmap{^^optionalFields, 0}]]
    std::string_view memo;
    
    [[=aitvaras::non_fixed_bitmap{^^optionalFields, 1}]]
    uint8_t orderType;
};

TEST(MarshalerTest, ForEachMethods) {
    char buffer[256] = {0};
    Marshaler<TestMessage> proxy{{std::span<char>(buffer, sizeof(buffer))}};
    
    proxy.msgType = 1;
    proxy.length = 10;
    proxy.optionalFields = 0; // all bits 0 initially
    EXPECT_TRUE((proxy.memo = "ABC").has_value()); // optional (tlv)
    EXPECT_TRUE((proxy.orderType = 5).has_value()); // optional (bitmap)
    
    std::vector<std::string> fixed_fields;
    proxy.for_each_fixed([&](std::string_view name, auto&& val) {
        fixed_fields.push_back(std::string(name));
    });
    
    EXPECT_EQ(fixed_fields.size(), 3u);
    EXPECT_EQ(fixed_fields[0], "msgType");
    EXPECT_EQ(fixed_fields[1], "length");
    EXPECT_EQ(fixed_fields[2], "optionalFields");

    std::vector<std::string> optional_fields;
    proxy.for_each_optional([&](std::string_view name, auto&& val) {
        optional_fields.push_back(std::string(name));
    });
    
    EXPECT_EQ(optional_fields.size(), 2u);
    EXPECT_EQ(optional_fields[0], "memo");
    EXPECT_EQ(optional_fields[1], "orderType");

    std::vector<std::string> all_fields;
    proxy.for_each_all([&](std::string_view name, auto&& val) {
        all_fields.push_back(std::string(name));
    });
    
    EXPECT_EQ(all_fields.size(), 5u);
    EXPECT_EQ(all_fields[0], "msgType");
    EXPECT_EQ(all_fields[1], "length");
    EXPECT_EQ(all_fields[2], "optionalFields");
    EXPECT_EQ(all_fields[3], "memo");
    EXPECT_EQ(all_fields[4], "orderType");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

struct [[=aitvaras::big_endian{}]] BigEndianTestMsg {
    uint32_t a;
    [[=aitvaras::little_endian{}]] uint16_t b;
};

TEST(MarshalerTest, BigEndianStruct) {
    char buffer[6] = {0};
    aitvaras::Marshaler<BigEndianTestMsg> proxy{{std::span<char>(buffer, sizeof(buffer))}};
    proxy.a = 0xAABBCCDD; // Should be swapped to DD CC BB AA since native is little endian
    proxy.b = 0x1122;     // Should NOT be swapped since native is little endian

    EXPECT_EQ(static_cast<uint8_t>(buffer[0]), 0xAA);
    EXPECT_EQ(static_cast<uint8_t>(buffer[1]), 0xBB);
    EXPECT_EQ(static_cast<uint8_t>(buffer[2]), 0xCC);
    EXPECT_EQ(static_cast<uint8_t>(buffer[3]), 0xDD);
    
    EXPECT_EQ(static_cast<uint8_t>(buffer[4]), 0x22);
    EXPECT_EQ(static_cast<uint8_t>(buffer[5]), 0x11);
    
    uint32_t val_a = proxy.a;
    EXPECT_EQ(val_a, 0xAABBCCDD);
    
    uint16_t val_b = proxy.b;
    EXPECT_EQ(val_b, 0x1122);
}
