# Copyright (c) 2026 Zeeshan Qazi
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

def process(filepath):
    with open(filepath, "r") as f:
        content = f.read()

    # OUCH Appendages are now properly typed. Replace spans with primitives.

    # appendageSecondaryOrdRefNum
    content = re.sub(r'const uint8_t (secondaryOrdRefNum|secondary|sec_ref)\[8\] = \{[0,]*([^}]+)\};\n\s*EXPECT_TRUE\(\(o?u?c?h?\.appendageSecondaryOrdRefNum = std::span<const uint8_t>\(\1, 8\)\)\.has_value\(\)\);',
                     lambda m: f'EXPECT_TRUE((o.appendageSecondaryOrdRefNum = {m.group(2)}).has_value());' if 'o.' in m.group(0) else f'EXPECT_TRUE((ouch.appendageSecondaryOrdRefNum = {m.group(2)}).has_value());', content)
    content = re.sub(r'EXPECT_EQ\(\(\*(sec|secondary)\)\[7\], ([^)]+)\);', r'EXPECT_EQ(*\1, \2);', content)
    content = re.sub(r'ASSERT_EQ\((sec|secondary)->size\(\), 8u\);\n\s*', '', content)

    # appendageFirm
    content = re.sub(r'const uint8_t (firm_data|firm1|firm2|firm|f1)\[\]\s*= \{\'([^\']+)\', \'([^\']+)\', \'([^\']+)\', \'([^\']+)\'\};\n\s*EXPECT_TRUE\(\(o?u?c?h?\.appendageFirm\s*= std::span<const uint8_t>\(\1,\s*4\)\)\.has_value\(\)\);',
                     lambda m: f'EXPECT_TRUE((o.appendageFirm = "{m.group(2)}{m.group(3)}{m.group(4)}{m.group(5)}").has_value());' if 'o.' in m.group(0) else f'EXPECT_TRUE((ouch.appendageFirm = "{m.group(2)}{m.group(3)}{m.group(4)}{m.group(5)}").has_value());', content)
    content = re.sub(r'EXPECT_EQ\(\(\*firm\)\[0\], \'([^\']+)\'\);\n\s*EXPECT_EQ\(\(\*firm\)\[3\], \'([^\']+)\'\);', 
                     lambda m: f'EXPECT_EQ((*firm)[0], \'{m.group(1)}\'); EXPECT_EQ((*firm)[3], \'{m.group(2)}\'); /* NEEDS MANUAL FIX */', content)
    content = re.sub(r'EXPECT_EQ\(\(\*fm\)\[0\], \'([^\']+)\'\);\n\s*EXPECT_EQ\(\(\*fm\)\[3\], \'([^\']+)\'\);', 
                     lambda m: f'EXPECT_EQ((*fm)[0], \'{m.group(1)}\'); EXPECT_EQ((*fm)[3], \'{m.group(2)}\'); /* NEEDS MANUAL FIX */', content)
    content = re.sub(r'EXPECT_EQ\(\(\*firm\)\[0\], \'([^\']+)\'\);\n\s*EXPECT_EQ\(\(\*firm\)\[1\], \'([^\']+)\'\);\n\s*EXPECT_EQ\(\(\*firm\)\[2\], \'([^\']+)\'\);\n\s*EXPECT_EQ\(\(\*firm\)\[3\], \'([^\']+)\'\);',
                     lambda m: f'EXPECT_EQ(*firm, "{m.group(1)}{m.group(2)}{m.group(3)}{m.group(4)}");', content)

    # appendageRoute
    content = re.sub(r'const uint8_t (route_data|route1|route2|route)\[\]\s*= \{\'([^\']+)\', \'([^\']+)\', \'([^\']+)\', \'([^\']+)\'\};\n\s*EXPECT_TRUE\(\(o?u?c?h?\.appendageRoute\s*= std::span<const uint8_t>\(\1,\s*4\)\)\.has_value\(\)\);',
                     lambda m: f'EXPECT_TRUE((o.appendageRoute = "{m.group(2)}{m.group(3)}{m.group(4)}{m.group(5)}").has_value());' if 'o.' in m.group(0) else f'EXPECT_TRUE((ouch.appendageRoute = "{m.group(2)}{m.group(3)}{m.group(4)}{m.group(5)}").has_value());', content)
    content = re.sub(r'EXPECT_EQ\(\(\*route\)\[0\], \'([^\']+)\'\);\n\s*EXPECT_EQ\(\(\*route\)\[3\], \'([^\']+)\'\);', 
                     lambda m: f'EXPECT_EQ((*route)[0], \'{m.group(1)}\'); EXPECT_EQ((*route)[3], \'{m.group(2)}\'); /* NEEDS MANUAL FIX */', content)
    content = re.sub(r'EXPECT_EQ\(\(\*route\)\[0\], \'([^\']+)\'\);\n\s*EXPECT_EQ\(\(\*route\)\[2\], \'([^\']+)\'\);\n\s*EXPECT_EQ\(\(\*route\)\[3\], \'([^\']+)\'\);', 
                     lambda m: f'EXPECT_EQ((*route)[0], \'{m.group(1)}\'); /* NEEDS MANUAL FIX */', content)
    content = re.sub(r'EXPECT_EQ\(\(\*rt\)\[0\], \'([^\']+)\'\);\n\s*EXPECT_EQ\(\(\*rt\)\[3\], \'([^\']+)\'\);', 
                     lambda m: f'EXPECT_EQ((*rt)[0], \'{m.group(1)}\'); EXPECT_EQ((*rt)[3], \'{m.group(2)}\'); /* NEEDS MANUAL FIX */', content)

    # appendageMinQty
    content = re.sub(r'const uint8_t (minqty1|minqty_data|minqty2)\[\] = \{[0x,]*([^}]+)\};\s*(?://.*)?\n\s*EXPECT_TRUE\(\(o?\.appendageMinQty = std::span<const uint8_t>\(\1,\s*4\)\)\.has_value\(\)\);',
                     lambda m: f'EXPECT_TRUE((o.appendageMinQty = {m.group(2)}).has_value());', content)
    content = re.sub(r'EXPECT_EQ\(\(\*minqty\)\[3\], ([^)]+)\);', r'EXPECT_EQ(*minqty, \1);', content)

    # appendagePegOffset
    content = re.sub(r'const uint8_t old_peg\[\] = \{0x01, 0x02\};\n\s*EXPECT_TRUE\(\(o.appendagePegOffset = std::span<const uint8_t>\(old_peg, 2\)\)\.has_value\(\)\);',
                     'EXPECT_TRUE((o.appendagePegOffset = 0x0102).has_value());', content)
    content = re.sub(r'const uint8_t new_peg\[\] = \{0xAA, 0xBB, 0xCC, 0xDD, 0xEE\};\n\s*EXPECT_TRUE\(\(o.appendagePegOffset = std::span<const uint8_t>\(new_peg, 5\)\)\.has_value\(\)\);',
                     'EXPECT_TRUE((o.appendagePegOffset = 0xCCDDEE).has_value());', content)
    content = re.sub(r'ASSERT_EQ\(peg->size\(\), 5u\);\n\s*EXPECT_EQ\(\(\*peg\)\[0\], 0xAAu\);\n\s*EXPECT_EQ\(\(\*peg\)\[4\], 0xEEu\);',
                     'EXPECT_EQ(*peg, 0xCCDDEE);', content)

    # appendageHandleInst
    content = re.sub(r'const uint8_t (handle_data|inst_data|inst2|hi1|hi2)\[\]\s*= \{([^}]+)\};\n\s*EXPECT_TRUE\(\(o\.appendageHandleInst = std::span<const uint8_t>\(\1, 1\)\)\.has_value\(\)\);',
                     lambda m: f'EXPECT_TRUE((o.appendageHandleInst = {m.group(2)}).has_value());', content)
    content = re.sub(r'EXPECT_EQ\(\(\*(hi|inst)\)\[0\], ([^)]+)\);', r'EXPECT_EQ(*\1, \2);', content)

    # Generic span single element fix
    content = re.sub(r'EXPECT_EQ\(\(\*firm\)\[0\], \'([^\']+)\'\);', r'EXPECT_EQ(*firm, "\1"); /* CHECK */', content)

    with open(filepath, "w") as f:
        f.write(content)

process("test/test_protocols.cpp")
process("test/test_appendage_lifecycle.cpp")

