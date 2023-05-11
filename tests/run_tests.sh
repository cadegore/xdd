#!/bin/bash

#Setup functional tests 
XDD_EXE_PATH=$(which xdd)
XDD_REPO_PATH=$HOME/Repos/xdd
echo XDDTEST_XDD_EXE=$XDD_EXE_PATH > $XDD_REPO_PATH/tests/functional/test_config
echo XDDTEST_LOCAL_MOUNT=$HOME/test-dir/tests >> $XDD_REPO_PATH/tests/functional/test_config
echo XDDTEST_OUTPUT_DIR=$HOME/test-dir/logs >> $XDD_REPO_PATH/tests/functional/test_config

cp $XDD_REPO_PATH/tests/functional/test_config ./test_config
cp $XDD_REPO_PATH/tests/functional/common.sh ./common.sh

#Run functional tests
set -e
$XDD_REPO_PATH/tests/functional/test_xdd_createnewfiles2.sh
rc=$?
echo $rc
# for f in $XDD_REPO_PATH/tests/functional/*2.sh; do
#   bash "$f"
# done
