import re

def generate_tests():
    with open("test/protocols.hpp", "r") as f:
        content = f.read()
        
    struct_pattern = re.compile(r'struct\s+(\w+)\s*{([^}]*)};')
    structs = struct_pattern.findall(content)
    
    out = """#include <gtest/gtest.h>
#include <aitvaras/marshaler.hpp>
#include "protocols.hpp"

using namespace aitvaras;

"""
    for struct_name, body in structs:
        out += f"TEST(ProtocolGeneratedTest, {struct_name}) {{\n"
        out += f"    char buffer[1024] = {{0}};\n"
        out += f"    Marshaler<{struct_name}> msg{{{{std::span<char>(buffer, sizeof(buffer))}}}};\n\n"
        
        # We just try to get/set everything to ensure it compiles and works basically.
        fields = re.findall(r'(std::string_view|std::span<const uint8_t>|uint\d+_t)\s+(\w+);', body)
        for i, (ftype, field) in enumerate(fields):
            if "presenceBits" in field:
                # Handled automatically or implicitly
                continue
            if field == "messageType" or field == "type" or field == "packetType":
                out += f"    EXPECT_TRUE((msg.{field} = 'X').has_value());\n"
            elif field == "length" or field == "appendageLength":
                out += f"    msg.{field} = 100;\n" # Some fixed length
            elif ftype == "std::span<const uint8_t>":
                out += f"    EXPECT_TRUE((msg.{field} = std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(\"TEST\"), 4)).has_value());\n"
            elif ftype == "std::string_view":
                out += f"    EXPECT_TRUE((msg.{field} = \"A\").has_value());\n"
            else:
                out += f"    EXPECT_TRUE((msg.{field} = {i+1}).has_value());\n"
                
        out += "}\n\n"
        
    with open("test/test_protocols_generated.cpp", "w") as f:
        f.write(out)

if __name__ == "__main__":
    generate_tests()
