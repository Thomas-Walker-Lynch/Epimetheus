#!/usr/bin/env python3
from TM import TM, TMS, Features

def test_base_machine():
  print("\n--- Test 1: Base Machine (TM_SR_ND) ---")
  data = [10, 20, 30]
  
  # No features requested
  t = TMS(data) 
  print(f"Created Base Machine: {t.len()} items.")
  
  # 1. Step & Read (Should work)
  t.s()
  print(f"Step -> Val: {t.r()} (Expected: 20)")
  
  # 2. Try Append (Should FAIL)
  print("Attempting aR()...")
  try:
    t.aR(99)
    print("!! FAIL: aR() succeeded on Base Machine")
  except AttributeError:
    print(">> SUCCESS: Caught expected AttributeError (Feature missing)")

def test_ar_machine():
  print("\n--- Test 2: Feature Machine (TM_SR_ND_AR) ---")
  data = [10, 20, 30]
  
  # Feature requested
  t = TMS(data, features=[Features.APPEND_RIGHT])
  print(f"Created Feature Machine: {t.len()} items.")
  
  # 1. Step
  t.s()
  
  # 2. Append (Should WORK)
  print("Attempting aR(99)...")
  try:
    t.aR(99)
    print(f">> SUCCESS: aR() completed. New Len: {t.len()}")
    
    # Verify data
    print(f"Tape content (from head): {t.rn(5)}")
  except AttributeError as e:
    print(f"!! FAIL: {e}")

  # 3. Entanglement Check
  print("Entangling...")
  t2 = t.e()
  print("Attempting aR(100) on Clone...")
  try:
    t2.aR(100)
    print(f">> SUCCESS: Clone inherited aR feature. Len: {t2.len()}")
  except AttributeError:
    print("!! FAIL: Clone lost the feature!")

if __name__ == "__main__":
  test_base_machine()
  test_ar_machine()

