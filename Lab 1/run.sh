#!/bin/bash
echo "----------Make--------"
make; 

echo
echo "----------Creating Files----------"
# cat /dev/random | head -c <bytecount>
cat /dev/random | head -c 100000 > src_1.dat
cat /dev/random | head -c 1000000 > src_2.dat


echo  "----------Running steps----------"
echo "Step 3"
./1 src_1.dat dst_step3.dat
echo
echo "Step 4"
./2 src_1.dat dst_step4.dat
echo
echo "Step 5 - copying 100,000 Bytes file"
./3 src_1.dat dst_step5_small.dat
echo
echo "Step 5 - copying 1000,000 Bytes file"
./3 src_2.dat dst_step5_big.dat

make clean;
echo
