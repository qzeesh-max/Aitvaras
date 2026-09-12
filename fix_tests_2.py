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

def fix_file(filename):
    with open(filename, 'r') as f:
        content = f.read()

    # test_appendage_lifecycle.cpp
    content = re.sub(r'const uint8_t route2\[\]\s*= \{.*?\};\n\s*', '', content)
    content = content.replace('std::span<const uint8_t>(route2,  4)', '"ROUT"')
    
    content = re.sub(r'const uint8_t minqty2\[\] = \{.*?\};\n\s*', '', content)
    content = content.replace('std::span<const uint8_t>(minqty2, 4)', '20u')
    
    content = re.sub(r'const uint8_t firm2\[\]\s*= \{.*?\};\n\s*', '', content)
    # firm2 was already processed in the first script, but this matches if they weren't caught
    content = content.replace('std::span<const uint8_t>(firm2,   4)', '"FIRM"')

    content = re.sub(r'const uint8_t hi1\[\] = \{.*?\};\n\s*', '', content)
    content = content.replace('std::span<const uint8_t>(hi1, 1)', '\'A\'')
    
    content = re.sub(r'const uint8_t f1\[\]\s*= \{.*?\};\n\s*', '', content)
    content = content.replace('std::span<const uint8_t>(f1,  4)', '"F111"')

    content = re.sub(r'const uint8_t hi2\[\] = \{.*?\};\n\s*', '', content)
    content = content.replace('std::span<const uint8_t>(hi2, 1)', '\'B\'')

    # test_protocols.cpp
    content = re.sub(r'const uint8_t secondary\[\] = \{.*?\};\n\s*', '', content)
    content = content.replace('std::span<const uint8_t>(secondary, 8)', '0x99')
    
    content = re.sub(r'const uint8_t route\[\] = \{.*?\};\n\s*', '', content)
    content = content.replace('std::span<const uint8_t>(route, 4)', '"RTE1"')
    
    content = re.sub(r'const uint8_t firm\[\]\s*= \{.*?\};\n\s*', '', content)
    content = content.replace('std::span<const uint8_t>(firm,  4)', '"FRM1"')

    with open(filename, 'w') as f:
        f.write(content)

fix_file('test/test_protocols.cpp')
fix_file('test/test_appendage_lifecycle.cpp')

