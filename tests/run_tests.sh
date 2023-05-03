#!/bin/bash

#Setup functional tests 
XDD_EXE_PATH=$(which xdd)
echo XDDTEST_XDD_EXE=$XDD_EXE_PATH > tests/test_config
echo XDDTEST_TESTS_DIR=. >> tests/test_config
echo XDDTEST_LOCAL_MOUNT=$HOME/test-dir >> tests/test_config

cp tests/test_config tests/functional/test_config

#Run functional tests
for f in tests/functional/*.sh; do
  bash "$f" || break
done
