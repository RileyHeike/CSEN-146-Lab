#!/bin/bash
echo "----------Make--------"
make; 

echo
echo "----------Creating Files----------"
# cat /dev/random | head -c <bytecount>
cat /dev/random | head -c 100000 > src_1.dat
cat /dev/random | head -c 200000 > src_2.dat
cat /dev/random | head -c 300000 > src_3.dat


echo  "----------Running steps----------"
echo "Step 1"
./copyfile src_1.dat dst_1.dat src_2.dat dst_2.dat src_3.dat dst_3.dat
echo

echo "Step 2"
./request_times
echo

make clean;
echo
