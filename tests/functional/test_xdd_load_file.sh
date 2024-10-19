#!/usr/bin/env bash
#
# Description - test xdd seek load
#
# Validate the functionality of xdd seek load is able to load a file with or without
# numreqs. It should also overwrite the blocksize and reqsize if provided.
#
# Get absolute path to script
SCRIPT=${BASH_SOURCE[0]}
SCRIPTPATH=$(dirname "${SCRIPT}")

# Source the test configuration environment
source "${SCRIPTPATH}/../test_config"
source "${SCRIPTPATH}/../common.sh"

# Perform pre-test
initialize_test
test_dir="${XDDTEST_LOCAL_MOUNT}/${TESTNAME}"

# Function to load a file with optional numreqs, blocksize, and reqsize
seek_load_file() {
  local load_file=$1
  local numreqs=$2
  local blocksize=$3
  local reqsize=$4
  local cmd="${XDDTEST_XDD_EXE} -seek load ${load_file} -target /dev/null"

  # If the optional parameters are provided, add them to the command
  if [ -n "${numreqs}" ]; then
    cmd+=" -numreqs ${numreqs}"
  fi
  if [ -n "${blocksize}" ]; then
    cmd+=" -blocksize ${blocksize}"
  fi
  if [ -n "${reqsize}" ]; then
    cmd+=" -reqsize ${reqsize}"
  fi

  # Execute the command
  eval "${cmd}"
}

# Function to verify the output
verify_output() {
  local output=$1
  local expected_blocksize=$2
  local expected_reqsize=$3
  local expected_numreqs=$4

  # Extract the blocksize, reqsize, and numreqs from the output
   local blocksize
  blocksize=$(echo "$output" | grep -oP 'Blocksize in bytes, \K[0-9]+')
  local reqsize
  reqsize=$(echo "$output" | grep -oP 'Request size, [0-9]+, 1024-byte blocks, \K[0-9]+')
  local numreqs
  numreqs=$(echo "$output" | grep -oP 'Number of Operations, \K[0-9]+')

  # Verify the blocksize, reqsize, and numreqs with the expected values
  if [[ "$blocksize" != "$expected_blocksize" ]]; then
    echo "Blocksize verification failed: expected $expected_blocksize, got $blocksize"
    return 1
  fi

  if [[ "$reqsize" != "$expected_reqsize" ]]; then
    echo "Reqsize verification failed: expected $expected_reqsize, got $reqsize"
    return 1
  fi

  if [[ "$numreqs" != "$expected_numreqs" ]]; then
    echo "Numreqs verification failed: expected $expected_numreqs, got $numreqs"
    return 1
  fi
}

# Test 1: Default blocksize and reqsize with numreqs 10
seek_file="${test_dir}/fool.dat"
load_file="${test_dir}/fool.dat.T0.txt"

# Create a file with numreqs 10 using default blocksize and reqsize
output=$("${XDDTEST_XDD_EXE}" -seek save "${seek_file}" -target /dev/null -numreqs 10)

# Test cases for default blocksize and reqsize
# 1. Load the file with no numreqs, no blocksize, and no reqsize
output=$(seek_load_file "${load_file}")
verify_output "$output" 1024 131072 10

# 2. Load the file with reqsize, no blocksize, and numreqs less than seek file
output=$(seek_load_file "${load_file}" 5 "" 128)
verify_output "$output" 1024 131072 5

# 3. Load the file with blocksize, no reqsize, and no numreqs
output=$(seek_load_file "${load_file}" "" 4096 "")
verify_output "$output" 1024 131072 10

# 4. Load the file with no blocksize, no reqsize, and numreqs larger than seek file
output=$(seek_load_file "${load_file}" 20 "" "")
verify_output "$output" 1024 131072 20

# Clean up
rm -f "${seek_file}" "${load_file}"

# Test 2: blocksize 8192 and reqsize 256 with numreqs 5
# Create a file with numreqs 5, custom blocksize, and custom reqsize
output=$("${XDDTEST_XDD_EXE}" -seek save "${seek_file}" -target /dev/null -numreqs 5 -blocksize 8192 -reqsize 256)
verify_output "$output" 8192 256 5

# Test cases for custom blocksize and reqsize
# 1. Load the file with no numreqs, no blocksize, and no reqsize
output=$(seek_load_file "${load_file}")
verify_output "$output" 8192 256 5

# 2. Load the file with reqsize, no blocksize, and numreqs less than seek file
output=$(seek_load_file "${load_file}" 3 "" 128)
verify_output "$output" 8192 256 3

# 3. Load the file with blocksize, no reqsize, and no numreqs
output=$(seek_load_file "${load_file}" "" 4096 "")
verify_output "$output" 8192 256 5

# 4. Load the file with no blocksize, no reqsize, and numreqs larger than seek file
output=$(seek_load_file "${load_file}" 10 "" "")
verify_output "$output" 8192 256 10

# Clean up
rm -f "${seek_file}" "${load_file}"

# Output test result
finalize_test 0