#include <gtest/gtest.h>
#include <aitvaras/marshaler.hpp>
#include "protocols.hpp"

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
    auto res1 = (proxy.symbol = "AAPL");
    EXPECT_TRUE(res1.has_value());
    
    // Check that it's written (symbol is a padded_string of length 8)
    std::string_view sym = proxy.symbol;
    EXPECT_EQ(sym, "AAPL");
    
    // Set an arithmetic field
    auto res2 = (proxy.price = 150.5); // price is fixed_point<6>
    EXPECT_TRUE(res2.has_value());
    double price_opt = proxy.price.get();
    EXPECT_DOUBLE_EQ(price_opt, 150.5);
    
    // Set a variable length string
    auto res3 = (proxy.memo = "Some order memo");
    EXPECT_TRUE(res3.has_value());
    
    // Read it back
    std::string_view memo_val = proxy.memo;
    EXPECT_EQ(memo_val, "Some order memo");
    
    // Set a non_fixed_bitmap enum field
    auto res4 = (proxy.orderType = OrderType::Market);
    EXPECT_TRUE(res4.has_value());
    
    // Check that the bitmask was updated
    uint32_t opt_fields = proxy.optionalFields.get();
    EXPECT_EQ(opt_fields, (1 << 1)); // bit 1 should be set, memo uses length_precedes
    
    // Check that we can read it back
    auto order_type_val = proxy.orderType.get();
    EXPECT_TRUE(order_type_val.has_value());
    EXPECT_EQ(*order_type_val, static_cast<uint8_t>(OrderType::Market));
    
    // Check total size updated
    EXPECT_GT(proxy.state_.total_size_, 30);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
