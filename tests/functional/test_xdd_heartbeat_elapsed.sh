#!/bin/bash
#
# Source the test configuration environment
#
# Description: outputs the elapsed time since the start of run
# Verify heartbeat elapsed time options works
#
source ./test_config

# Pre-test set-up
test_name=$(basename -s .sh $0)
test_name="${test_name%.*}"
test_dir=$XDDTEST_LOCAL_MOUNT/$test_name

mkdir -p $test_dir

test_file=$test_dir/data1
touch $test_file

data_loc=$test_dir/data2.T0000.csv
delay_time=3
passes=3
theoretical_seconds=$(($delay_time * $passes))


$XDDTEST_XDD_EXE -target $test_file -op write -passes $passes -reqsize 1 -numreqs 1 -startdelay $delay_time -hb elapsed -hb output $test_dir/data2 -hb lf


actual_seconds=$(cat $data_loc | wc -l)
error="$(($actual_seconds-$theoretical_seconds))"
abs_error=${error#-}
percent_error=$(echo "scale=2 ; $abs_error / $theoretical_seconds * 100" | bc)
error_bound=20

pass_count=0
if (( $(echo "$percent_error < $error_bound" | bc -l) )); then
  pass_count=1
fi
# Post test cleanup
rm -r $test_dir

if [[ $pass_count -eq 1 ]]; then
  echo "Heartbeat elapsed test PASSED"
  exit 0
fi

echo "Heartbeat elapsed test FAILED"
exit 1

