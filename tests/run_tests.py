#!/bin/python3 
import subprocess
import os
import time
from pathlib import Path
import glob

rc_values = {
        -1: "SKIPPED",
        0 : "PASSED",
        1 : "FAILED"
        }

def main():
    setup()
    run_functional_tests()

def setup():
    global xdd_exe_location
    xdd_exe_location = subprocess.run(['which','xdd'], universal_newlines=True, stdout=subprocess.PIPE).stdout.strip('\n')

    #location of xdd github repo
    global xdd_repo_location
    xdd_repo_location = Path.home() / 'Repos' / 'xdd'
    
    #location of config file for running functional tests
    global xdd_test_config_location
    xdd_test_config_location = xdd_repo_location / 'tests' / 'functional'
    
    #location of where functional tests will run
    global xdd_test_localmount_location
    xdd_test_localmount_location = Path.home() / 'test-dir' / 'tests'

    #location of where functional tests will output logs to
    global xdd_test_output_dir
    xdd_test_output_dir = Path.home() / 'test-dir' / 'logs'

    with open(xdd_test_config_location / 'test_config', 'w') as f:
        f.write(f"XDDTEST_XDD_EXE={xdd_exe_location}\n")
        f.write(f"XDDTEST_LOCAL_MOUNT={xdd_test_localmount_location}\n")
        f.write(f"XDDTEST_OUTPUT_DIR={xdd_test_output_dir}\n")

def run_functional_tests():
    os.chdir(xdd_repo_location / 'tests' / 'functional')
    list_of_tests = glob.glob('test*.sh')

    test_results = {}

    #run each test in functional folder, recording the return value and time it took to execute
    for test in list_of_tests:
        test_log_filename = test.strip('.sh')
        test_log_filename_file = open(xdd_test_output_dir / test_log_filename, 'w')
        start = time.time()
        rc = subprocess.run(['bash',test], universal_newlines=True,stderr=subprocess.STDOUT,stdout=test_log_filename_file).returncode
        end = time.time()
        test_log_filename_file.close()
        test_results[test] = [rc, round((end - start),2)]

    #array to store outcomes of tests 0 -> pass, 1 -> failed, -1 -> skipped
    summary = [0, 0, 0]
    for test in test_results:
        print(f"{test} ------ {rc_values[test_results[test][0]]} ------ {test_results[test][1]} seconds")
        rc = test_results[test][0]
        
        if (rc == 0 ):
            summary[0] += 1
        elif (rc == 1):
            summary[1] += 1
        elif (rc == -1):
            summary[2] += 1

    print(f"Passes: {summary[0]}")
    print(f"Failures: {summary[1]}")
    print(f"Skipped: {summary[2]}")


    
if __name__ == "__main__":
    main()
