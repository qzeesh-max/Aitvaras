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

    # test_appendage_lifecycle.cpp remaining errors
    content = re.sub(r'EXPECT_EQ\(\(\*minqty\)\[3\], 0x14u\);', 'EXPECT_EQ(*minqty, 20u);', content)
    content = re.sub(r'EXPECT_EQ\(\(\*hi\)\[0\], \'A\'\);', 'EXPECT_EQ(*hi, \'A\');', content)
    content = re.sub(r'EXPECT_EQ\(\(\*hi\)\[0\], \'B\'\);', 'EXPECT_EQ(*hi, \'B\');', content)

    with open(filename, 'w') as f:
        f.write(content)

fix_file('test/test_appendage_lifecycle.cpp')
