#!/bin/bash

#Setup functional tests 
echo XDD_EXE_PATH=$HOME/Software/xdd/bin/xdd > ./test_config
echo XDDTEST_XDD_EXE=$XDD_EXE_PATH >> ./test_config
echo XDDTEST_TESTS_DIR=. >> ./test_config
echo XDDTEST_LOCAL_MOUNT=$HOME/test-dir >> ./test_config

# mv ./test_config ./tests/functional/test_config

#Run functional tests
set -e
status=0
for f in tests/functional/*.sh; do
  bash "$f"
  status=$(($status + $?)) 
done
