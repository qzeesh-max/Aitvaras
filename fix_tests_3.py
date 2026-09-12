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
