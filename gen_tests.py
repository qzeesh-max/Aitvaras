# Copyright (c) 2026 Zeeshan Qazi <zeeshan@zeeshan.im>
#
# This file is part of Aitvaras.
#
# Aitvaras is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Aitvaras is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with Aitvaras.  If not, see <https://www.gnu.org/licenses/>.

import re

def generate_tests():
    headers = {
        'protocol_rake.hpp': 'Rake',
        'protocol_soup.hpp': 'Soup',
        'protocol_ouch.hpp': 'Ouch',
        'protocol_seed.hpp': 'Seed'
    }
    
    for header, prefix in headers.items():
        with open(f"test/{header}", "r") as f:
            content = f.read()
            
        struct_pattern = re.compile(r'struct\s+(\w+)\s*{([^}]*)};')
        structs = struct_pattern.findall(content)
        
        if not structs:
            continue
            
        out = f"""#include <gtest/gtest.h>
#include <aitvaras/marshaler.hpp>
#include \"{header}\"

using namespace aitvaras;

"""
        for struct_name, body in structs:
            out += f"TEST({prefix}GeneratedTest, {struct_name}) {{\n"
            out += f"    char buffer[1024] = {{0}};\n"
            out += f"    Marshaler<{struct_name}> msg{{{{std::span<char>(buffer, sizeof(buffer))}}}};\n\n"
            
            # We just try to get/set everything to ensure it compiles and works basically.
            fields = re.findall(r'(std::string_view|std::span<const uint8_t>|uint\d+_t)\s+(\w+);', body)
            for i, (ftype, field) in enumerate(fields):
                if "presenceBits" in field:
                    # Handled automatically or implicitly
                    continue
                if field == "messageType" or field == "type" or field == "packetType":
                    out += f"    msg.{field} = 'X';\n"
                elif field == "length" or field == "appendageLength":
                    out += f"    msg.{field} = 100;\n" # Some fixed length
                elif ftype == "std::span<const uint8_t>":
                    out += f"    EXPECT_TRUE((msg.{field} = std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(\"TEST\"), 4)).has_value());\n"
                elif ftype == "std::string_view":
                    out += f"    msg.{field} = \"A\";\n"
                else:
                    out += f"    msg.{field} = {i+1};\n"
                    
            out += "}\n\n"
            
        with open(f"test/test_{header.replace('.hpp', '_generated.cpp')}", "w") as f:
            f.write(out)

if __name__ == "__main__":
    generate_tests()
