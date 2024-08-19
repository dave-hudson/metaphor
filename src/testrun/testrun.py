import json
import subprocess
import os
import sys
import argparse
import concurrent.futures
from pathlib import Path
from multiprocessing import cpu_count

DEFAULT_TIMEOUT = 5000  # milliseconds

def parse_config_file(config_file):
    with open(config_file, 'r') as file:
        try:
            config = json.load(file)
            for test in config:
                validate_test_config(test)
            return config
        except json.JSONDecodeError as e:
            print(f"Error parsing the configuration file: {e}")
            sys.exit(1)

def validate_test_config(test):
    mandatory_keys = {"command", "type"}
    valid_types = {"positive", "negative"}
    
    for key in test:
        if key not in {"command", "type", "expected", "timeout"}:
            print(f"Invalid key '{key}' found in the configuration.")
            sys.exit(1)

    if not mandatory_keys.issubset(test.keys()):
        print(f"Mandatory keys missing in test configuration: {mandatory_keys - set(test.keys())}")
        sys.exit(1)
    
    if test["type"] not in valid_types:
        print(f"Invalid test type '{test['type']}' in configuration.")
        sys.exit(1)

def run_test(test):
    command = test["command"]
    test_type = test["type"]
    timeout = test.get("timeout", DEFAULT_TIMEOUT)
    expected_file = test.get("expected", None)
    
    print(f"Running test: {command}")
    
    try:
        process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        stdout, _ = process.communicate(timeout=timeout / 1000)
        exit_code = process.returncode

        stdout = stdout.decode('utf-8')
        
        if test_type == "positive" and exit_code != 0:
            return "FAIL", command, stdout
        elif test_type == "negative" and exit_code == 0:
            return "FAIL", command, stdout
        
        if expected_file:
            with open(expected_file, 'r') as file:
                expected_output = file.read()
                if stdout != expected_output:
                    return "FAIL", command, stdout
        return "PASS", command, stdout
        
    except subprocess.TimeoutExpired:
        process.kill()
        return "FAIL", command, f"Test timed out after {timeout} ms"
    except Exception as e:
        return "FAIL", command, str(e)

def execute_tests(config, max_parallel_tests):
    with concurrent.futures.ThreadPoolExecutor(max_workers=max_parallel_tests) as executor:
        futures = {executor.submit(run_test, test): test for test in config}
        results = []
        for future in concurrent.futures.as_completed(futures):
            result = future.result()
            results.append(result)
            status, command, _ = result
            print(f"{status}: {command}")
        return results

def summarize_results(results):
    total_tests = len(results)
    passed_tests = sum(1 for result in results if result[0] == "PASS")
    failed_tests = total_tests - passed_tests
    print("\nTest Summary:")
    print(f"Total tests: {total_tests}")
    print(f"Passed: {passed_tests}")
    print(f"Failed: {failed_tests}")
    return failed_tests == 0

def main():
    parser = argparse.ArgumentParser(description="Run tests for the metaphorc compiler.")
    parser.add_argument("config_file", help="Path to the JSON configuration file.")
    parser.add_argument("--parallel-tests", type=int, default=cpu_count(), help="Maximum number of parallel tests.")
    
    args = parser.parse_args()
    
    if not Path(args.config_file).is_file():
        print(f"Configuration file '{args.config_file}' does not exist.")
        sys.exit(1)
    
    config = parse_config_file(args.config_file)
    results = execute_tests(config, args.parallel_tests)
    
    all_tests_passed = summarize_results(results)
    
    sys.exit(0 if all_tests_passed else 1)

if __name__ == "__main__":
    main()

