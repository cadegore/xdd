#!/usr/bin/env bash
#
# Acceptance test for XDD.
#
# Validate the funtionality of -datapattern file/wholefile option by creating a file, setting the datapattern
# from that file and then verifying the contents match
#
# Description - terminates XDD after a given amount of seconds have passed
#
# Get absolute path to script
SCRIPT=${BASH_SOURCE[0]}
SCRIPTPATH=$(dirname "${SCRIPT}")

# Source the test configuration environment
source "${SCRIPTPATH}"/../test_config
source "${SCRIPTPATH}"/../common.sh 

# Perform pre-test
initialize_test
test_dir="${XDDTEST_LOCAL_MOUNT}/${TESTNAME}"
log_file="$(get_log_file)"

input_file="${test_dir}/data1.dat"
output_file="${test_dir}/data2.dat"

# Create datapattern input file using dd
dd if=/dev/urandom of="${input_file}" bs=6k count=1

# Write out file using xdd with -datapattern file
"${XDDTEST_XDD_EXE}" -op write -reqsize 7 -numreqs 1 -datapattern wholefile "${input_file}" -targets 1 "${output_file}" -qd 1 -passes 1
# Appending rest of data from buffer.iso to buffer.iso without truncating the file
dd if="${input_file}" of="${input_file}" bs=1k count=1 oflag=append conv=notrunc

# Verify the contents of the file match
if ! cmp "${input_file}" "${output_file}"
then
    echo "Error when comparing files ${input_file} and ${output_file} with -datapattern file" > "${log_file}"
    cmp "${input_file}" "${output_file}" >> "${log_file}"
    rm "${input_file}" "${output_file}"
    finalize_test 1 "Issue with setting datapattern from file with -datapattern file. Contents don't match."
fi

rm "${input_file}" "${output_file}"

# Create datapattern input file using dd
dd if=/dev/urandom of="${input_file}" bs=5k count=2

# Write out file using xdd with -datapattern wholefile
# In this caes we will use two threads to write out the data in serial ordering as the
# contents of the file will be distributed evenly between both threads.
"${XDDTEST_XDD_EXE}" -op write -reqsize 7 -numreqs 2 -datapattern wholefile "${input_file}" -targets 1 "${output_file}" -serialordering -qd 9 -passes 1
# Appending rest of data from buffer.iso to buffer.iso without truncating the file
dd if="${input_file}" of="${input_file}" bs=2k count=2 oflag=append conv=notrunc
# Verify the contents of the file match
if ! cmp "${input_file}" "${output_file}"
then
    echo "Error when comparing files ${input_file} and ${output_file} with -datapattern wholefile" > "${log_file}"
    cmp "${input_file}" "${output_file}" >> "${log_file}"
    rm "${input_file}" "${output_file}"
    finalize_test 1 "Issue with setting datapattern from file with -datapattern file. Contents don't match."
fi
rm "${input_file}" "${output_file}"

# Test passed
finalize_test 0
