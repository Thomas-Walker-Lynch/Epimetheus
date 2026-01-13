#!/usr/bin/env python3
import os
import sys
# Ensure we can import rt_fmt from the current directory
sys.path.append(os.getcwd())
from rt_fmt import process_file

def run_test(test_id ,lang_ext):
  """
  Work Function: Orchestrates a single file test and reports diffs.
  """
  in_fp = f"test_{test_id}_in.{lang_ext}"
  exp_fp = f"test_{test_id}_expected.{lang_ext}"
  out_fp = f"test_{test_id}_out.{lang_ext}"

  if not os.path.exists(in_fp):
    print(f"Skipping Test {test_id}: {in_fp} not found.")
    return False

  # Call the RT Formatter Work Function
  # Pass distinct Input and Output paths to prevent clobbering.
  process_file(in_fp, out_fp)

  # Compare results
  if not os.path.exists(exp_fp):
      print(f"Warning: No expectation file found for {in_fp}")
      return False

  with open(exp_fp ,'r') as f_exp ,open(out_fp ,'r') as f_out:
    exp_lines = f_exp.readlines()
    out_lines = f_out.readlines()

  # Skip header line (Filename comments will naturally differ)
  if len(exp_lines) > 0 and len(out_lines) > 0:
    exp_data = exp_lines[1:]
    out_data = out_lines[1:]
  else:
    exp_data = exp_lines
    out_data = out_lines

  success_flag = (exp_data == out_data)

  if success_flag:
    print(f"PASS: Test {test_id} ({lang_ext})")
  else:
    print(f"FAIL: Test {test_id} ({lang_ext})")
    for i ,(e ,o) in enumerate(zip(exp_data ,out_data)):
      if e != o:
        # Use line index + 2 because we skipped line 0 (header) and are 0-indexed
        print(f"  Line {i+2} mismatch:")
        print(f"    Expected: {repr(e)}")
        print(f"    Result:   {repr(o)}")
  
  return success_flag

def CLI():
  """
  CLI Function: Runs the test suite.
  """
  print("Starting RT Formatter Test Suite...")
  results_seq = [
    run_test(0 ,"c")
    ,run_test(1 ,"py")
  ]

  total_count = len(results_seq)
  pass_count = sum(1 for r in results_seq if r)
  
  print(f"\nSummary: {pass_count}/{total_count} tests passed.")
  if pass_count < total_count:
    sys.exit(1)

if __name__ == "__main__":
  CLI()
