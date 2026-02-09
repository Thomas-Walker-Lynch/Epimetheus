#!/usr/bin/env python3
from TM import TM

def test_base_machine():
  print("--- TM·Array_SR_ND Verification ---")

  data = [10, 20, 30]
  tm = TM(data)
  print(f"Created TM. Data: {data}")
  
  # Forward Scan
  print("\n[Forward Scan]")
  while True:
    print(f"Read: {tm.r()}")
    if tm.qR(): break
    tm.s()
    
  # Rewind
  print("\n[Rewind]")
  tm.LsR()
  print(f"After LsR, Read: {tm.r()} (Expected: 10)")

  # Write
  tm.w(999)
  print(f"Wrote 999. Read: {tm.r()} (Expected: 999)")

if __name__ == "__main__":
  test_base_machine()
