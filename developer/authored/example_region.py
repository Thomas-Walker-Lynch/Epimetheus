#!/usr/bin/env python3
from TM import TMS, RegionMachine

def test_region():
  print("--- Region Machine Verification ---")
  
  # Tape: [10, 20, 30, 40, 50]
  t = TMS([10, 20, 30, 40, 50])
  
  # Create Region. Initially Boundaries are at 0 (start).
  # Region is [0, 0] (Single Cell).
  rm = RegionMachine(t)
  
  print("Region Initialized [0, 0].")
  print(f"qL: {rm.qL()} (Exp: True)")
  print(f"qR: {rm.qR()} (Exp: True)")
  
  # Expand Region: Move Right Boundary to index 2
  print("Expanding Right Boundary to index 2...")
  rm.t_right.sn(2) 
  
  # Region is now [0, 2] (Indices 0, 1, 2)
  print(f"qR (Active at 0): {rm.qR()} (Exp: False)")
  
  # Move Active Head
  rm.s() # To 1
  print(f"Active at 1. Val: {rm.r()}")
  rm.s() # To 2
  print(f"Active at 2. Val: {rm.r()}")
  
  print(f"qR (Active at 2): {rm.qR()} (Exp: True)")
  
  # Try to step out of region
  print("Attempting to step past Right Boundary...")
  try:
    rm.s()
    print("!! FAIL: Stepped out of region!")
  except RuntimeError as e:
    print(f">> SUCCESS: Blocked by Boundary. ({e})")

if __name__ == "__main__":
  test_region()
  
