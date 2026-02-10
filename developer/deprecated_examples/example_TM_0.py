#!/usr/bin/env python3
import sys
from TM import TM ,Topology

def CLI():
  print("--- TM Happy Path (example_TM_0) ---")

  # 1. Initialization
  # ---------------------------------------------------------
  print("\n[1] Initialization")
  data_list = [10 ,20 ,30 ,40 ,50]
  tm_0 = TM(data_list)
  print(f"Created TM with: {data_list}")
  print(f"Address: {tm_0.address()} (Should be 0)")
  print(f"Length:  {tm_0.len()} (Should be 5)")

  # 2. Read / Step / Write
  # ---------------------------------------------------------
  print("\n[2] Movement and IO")
  
  # Read current (0)
  val = tm_0.r()
  print(f"r(): {val} (Exp: 10)")
  
  # Step 1, Read
  tm_0.s()
  print(f"s() -> r(): {tm_0.r()} (Exp: 20)")
  
  # Write
  tm_0.w(99)
  print(f"w(99) -> r(): {tm_0.r()} (Exp: 99)")
  
  # Step N
  tm_0.sn(2)
  print(f"sn(2) -> r(): {tm_0.r()} (Exp: 40)") # 0->1->3 (Indices: 0, 1, 2, 3) 
  
  # Step Left
  tm_0.ls()
  print(f"ls() -> r(): {tm_0.r()} (Exp: 30)")
  
  # Bulk Write (wn)
  # Current head is at index 2 (val 30)
  # Write [33, 44] -> overwrites 30, 40
  tm_0.wn([33 ,44])
  print("wn([33 ,44])")
  
  # Bulk Read (rn)
  read_back = tm_0.rn(2)
  print(f"rn(2): {read_back} (Exp: [33, 44])")

  # 3. Allocation
  # ---------------------------------------------------------
  print("\n[3] Allocation")
  
  # Append Right (aR)
  tm_0.aR(60)
  print(f"aR(60) -> len: {tm_0.len()} (Exp: 6)")
  
  # Append Left (aL)
  # Head is at index 2. 
  # aL inserts at 0. Indices shift right.
  # Head should increment to 3 to stay on '33'.
  print(f"Pre-aL Head: {tm_0.address()}")
  tm_0.aL(0)
  print(f"aL(0) -> Head: {tm_0.address()} (Exp: 3)")
  print(f"Value at Head: {tm_0.r()} (Exp: 33)")
  
  # Check 0 index
  tm_0.lsn(3) # Go to 0
  print(f"Value at 0: {tm_0.r()} (Exp: 0)")

  # 4. Deletion
  # ---------------------------------------------------------
  print("\n[4] Deletion")
  
  # Current Tape: [0, 10, 99, 33, 44, 50, 60]
  # Head at 0.
  
  # esd (Delete Neighbor -> 10)
  tm_0.esd()
  print(f"esd() -> Tape[1] should be 99. r(2): {tm_0.rn(2)} (Exp: [0, 99])")
  
  # d (Delete Current -> 0)
  tm_0.d()
  # Head stays at 0, which is now 99
  print(f"d() -> Current: {tm_0.r()} (Exp: 99)")

  # 5. Cloning (Entanglement)
  # ---------------------------------------------------------
  print("\n[5] Cloning")
  tm_clone = tm_0.e()
  print("Created tm_clone from tm_0")
  
  # Modify tm_0, check tm_clone
  tm_0.w(999)
  print(f"tm_0.w(999)")
  print(f"tm_clone.r(): {tm_clone.r()} (Exp: 999)")

  print("\n--- end of examples ---")

if __name__ == "__main__":
  CLI()

  
