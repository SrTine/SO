#!/bin/bash

MPOINT="./mount-point"
BLOCK_SIZE=4096

rm -R -f temp
mkdir temp

echo "Copying fuseLib.c and myFS.h"
cp ./src/fuseLib.c ./temp/file1
cp ./src/myFS.h ./temp/file2
cp ./temp/* $MPOINT/
read -p "Press enter..."

echo "Auditing disk"
./my-fsck virtual-disk
read -p "Press enter..."

echo "Comparing original files and SF files"
diff ./temp/file1 $MPOINT/file1
diff ./temp/file2 $MPOINT/file2
read -p "Press enter..."

echo "Truncating file1"
truncate -s -$BLOCK_SIZE ./temp/file1
truncate -s -$BLOCK_SIZE $MPOINT/file1
read -p "Press enter..."

echo "Auditing disk"
./my-fsck virtual-disk
read -p "Press enter..."

echo "Comparing truncated file1"
diff ./temp/file1 $MPOINT/file1
read -p "Press enter..."

echo "Copying file3"
cp ./Makefile ./temp/file3
cp ./temp/file3 $MPOINT/file3
read -p "Press enter..."

echo "Auditing disk"
./my-fsck virtual-disk
read -p "Press enter..."

echo "Comparing file3"
diff ./temp/file3 $MPOINT/file3
read -p  "Press enter..."

echo "Truncate file2, adding size"
truncate -s +$BLOCK_SIZE ./temp/file2
truncate -s +$BLOCK_SIZE $MPOINT/file2
read -p "Press Enter..."

echo "Auditing disk"
./my-fsck virtual-disk
read -p "Press enter..."

echo "Comparing truncated file2"
diff ./temp/file2 $MPOINT/file2
read -p "Press enter..."

ls $MPOINT -la
read -p "Press enter..."

rm -R -f temp
