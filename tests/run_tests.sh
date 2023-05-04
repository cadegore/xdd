#!/bin/bash

#Setup functional tests 
XDD_EXE_PATH=$(which xdd)
echo XDDTEST_XDD_EXE=$XDD_EXE_PATH > ./test_config
echo XDDTEST_TESTS_DIR=. >> ./test_config
echo XDDTEST_LOCAL_MOUNT=$HOME/test-dir >> ./test_config

mv ./test_config ./tests/functional/test_config

#Run functional tests
for f in tests/functional/*.sh; do
  bash "$f" || break
done
