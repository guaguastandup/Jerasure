#!/bin/bash -e
#
# Copyright (C) 2014 Red Hat <contact@redhat.com>
#
# Author: Loic Dachary <loic@dachary.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library Public License for more details.

# 16MB:     16777216
# 4*16MB:   67108864
# 16*16MB:  268435456 读取256MB
# 18*16MB:  301989888
# 24*16MB:  402653184
# 12*16MB:  201326592
# dd if=/dev/urandom of=T bs=16M count=64

# [RAID6]:
# trap "rm -fr T Coding"  EXIT

# rm -rf Coding
# dd if=/dev/urandom of=T bs=16M count=32
# ./encoder T 4 2 RAID6 8 8 268435456
# cd Coding
# # mv T_k1 T_erased
# # mv T_k2 T_erased
# mv T_k3 T_erased
# # mv T_k4 T_erased
# cd ..
# ./decoder T
# cd Coding
# ls -lh
# ........................................................................
# ........................................................................
# [ROTATEDRS]:
rm -rf Coding
dd if=/dev/urandom of=T bs=16M count=48

./encoder T 6 3 rotated_rs 8 8 402653184

cd Coding && mv T_k1 T_erased
cd .. && ./decoder T

cd Coding && mv T_erased T_k1 && mv T_k2 T_erased
cd .. && ./decoder T

cd Coding && mv T_erased T_k2 && mv T_k3 T_erased
cd .. && ./decoder T

cd Coding && mv T_erased T_k3 && mv T_k4 T_erased
cd .. && ./decoder T

cd Coding && mv T_erased T_k4 && mv T_k5 T_erased
cd .. && ./decoder T

cd Coding && mv T_erased T_k5 && mv T_k6 T_erased
cd .. && ./decoder T

cd Coding && mv T_erased T_k6 && mv T_m1 T_erased
cd .. && ./decoder T

cd Coding && mv T_erased T_m1 && mv T_m2 T_erased
cd .. && ./decoder T

cd Coding && mv T_erased T_m2 && mv T_m3 T_erased
cd .. && ./decoder T


# ........................................................................
# ........................................................................
# [reed_sol]
# rm -rf Coding
# dd if=/dev/urandom of=T bs=4096 count=1
# # ./encoder T 4 3 reed_sol_van 8 0 0
# ./encoder T 4 2 reed_sol_r6_op 8 0 0
# ./decoder T