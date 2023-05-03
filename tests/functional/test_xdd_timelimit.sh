#!/bin/sh 
#
# Acceptance test for XDD.
#
# Validate the funtionality of -timelimit option by reading the test file forever untill the time restriction stops it
#
# Description - sets a time limit in seconds for each pass
#
# Source the test configuration environment
#
source ./test_config

test_name=$(basename -s sh $0)
test_name="${test_name%.*}" 

# Perform pre-test 
echo "Beginning xdd timelimit test . . ."
test_dir=$XDDTEST_LOCAL_MOUNT/$test_name
rm -rf $test_dir
mkdir -p $test_dir || exit_error

# ReqSize 4096, Bytes 1GB, Targets 1, QueueDepth 4, Passes 1
data_file=$test_dir/test
# write a file
$XDDTEST_XDD_EXE -op write -reqsize 4096 -mbytes 1024 -targets 1 $data_file -qd 4 -passes 1 -datapattern random 
# now read forever, small random I/O  with a timelimit

$XDDTEST_XDD_EXE -op read  -reqsize 1 -numreqs 999999 -targets 1 $data_file -qd 4 -timelimit 10.0  -passes 1 -seek random -seek range 1024 & 
pid=$!

sleep 15
# Validate output
test_passes=1

pkill $pid
pkill_status=$?
if [ $pkill_status -eq 0 ]; then
  test_passes=0
  echo "Had to kill $pid."
fi

# wait $pid

# Perform post-test cleanup
rm -rf $test_dir

# Output test result
echo -n "Acceptance Test - $test_name : "
if [ 1 -eq $test_passes ]; then
  echo "PASSED."
  exit 0
else
  echo "FAILED" 
  exit 1  
fi
