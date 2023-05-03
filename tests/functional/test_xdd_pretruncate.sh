#!/bin/bash
#
# Acceptance test for XDD
#
# Description - Truncate the file to a given length before doing I/O
#
# Validate -pretruncate by checking if the test file size is equal to the
#  truncation size
#
# Source the test configuration environment
#
source ./test_config

# Pre-test create test directory and file
test_name=$(basename -s sh $0)
test_name="${test_name%.*}"
test_dir=$XDDTEST_LOCAL_MOUNT/$test_name
mkdir -p $test_dir

test_file=$test_dir/data1

# Preallocate test file
pretruncate_blocks=4096
req_size=1
block_size=1024
$XDDTEST_XDD_EXE -target $test_file -op write -reqsize $req_size -numreqs 1 -bs $block_size -pretruncate $pretruncate_blocks 

pretruncate_size=$(($pretruncate_blocks * $block_size))
file_size=$(stat -c %s $test_file)

# Post test clean up
rm -r $test_dir

# Verify output
echo -n "Acceptance test - $test_name : "
if [ "$file_size" -eq "$pretruncate_size" ]; then
    echo "PASSED"
    exit 0
else
    echo "FAILED"
     echo "File size is $file_size, but pretruncate size was $pretruncate_size"
    exit 1
fi
