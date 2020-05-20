#!/usr/bin/env python3

import re
from collections import namedtuple

Region = namedtuple('Region', ['addr', 'size'])

regions = []

for line in open('poop.txt'):
    line.strip()
    match = re.search(r'^{"", (\w+), (\w+)},$', line)
    regions.append(Region(int(match.group(1), 16), int(match.group(2), 16)))

regions.sort(key=lambda reg: reg.addr)

new_regions = []

for reg in regions:
    if len(new_regions) == 0:
        new_regions.append(reg)
    else:
        last_reg_end = new_regions[-1].addr + new_regions[-1].size
        if last_reg_end == reg.addr:
            new_regions[-1] = Region(new_regions[-1].addr, new_regions[-1].size + reg.size)
        else:
            new_regions.append(reg)

total_size = sum(reg.size for reg in new_regions)
print(f"Total size: {total_size}")
for reg in new_regions:
    print('{{"", 0x{:X}, 0x{:X}}},'.format(reg.addr, reg.size))
