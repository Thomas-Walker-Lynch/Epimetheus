#!/usr/bin/env python3
import sys
from TM import TM

def CLI():
  print("--- TM Edge Cases (example_TM_1) ---")

  # 1. Entanglement Violation (Safety Check)
  # ---------------------------------------------------------
  print("\n[1] Entanglement Violation (Peer on Victim)")
  
  t1 = TM(['A' ,'B' ,'C'])
  t2 = t1.e() # Entangled Clone
  
  # Setup:
  # t1 at 0 ('A')
  # t2 moves to 1 ('B')
  t2.s() 
  print(f"Setup: t1@{t1.address()}, t2@{t2.address()}")
  
  print("Action: t1.esd() -> Tries to delete 'B'")
  print("Expect: RuntimeError (Entanglement Violation)")
  
  try:
    t1.esd()
    print("!! FAILED: Operation succeeded (Should have failed)")
  except RuntimeError as e:
    print(f">> CAUGHT EXPECTED ERROR: {e}")

  # 2. Entanglement Violation (Peer on Current)
  # ---------------------------------------------------------
  print("\n[2] Entanglement Violation (Peer on Current)")
  
  # t1 at 0. t2 at 1.
  # Move t2 back to 0. Both at 0.
  t2.ls()
  print(f"Setup: t1@{t1.address()}, t2@{t2.address()}")
  
  print("Action: t1.d() -> Tries to delete 'A'")
  print("Expect: RuntimeError")
  
  try:
    t1.d()
    print("!! FAILED: Operation succeeded")
  except RuntimeError as e:
    print(f">> CAUGHT EXPECTED ERROR: {e}")

  # 3. Safe Deletion (No Peer Collision)
  # ---------------------------------------------------------
  print("\n[3] Safe Deletion (Peer Safe)")
  
  # t1 at 0 ('A'), t2 at 0 ('A').
  # Move t2 to 2 ('C').
  t2.sn(2)
  print(f"Setup: t1@{t1.address()} ('A'), t2@{t2.address()} ('C')")
  
  # t1 deletes 'B' (neighbor). 'B' is at index 1.
  # t2 is at index 2. Safe?
  # Yes. t2 is not ON the cell being deleted.
  # Note: t2's data will shift left index-wise, but Entanglement check
  # only cares if t2 is *on* the deleted cell.
  
  print("Action: t1.esd() -> Delete 'B'")
  t1.esd()
  print(">> Success (Operation Permitted)")
  print(f"Tape is now: {t1.rn(2)}")

  # 4. Map Input (Materialization)
  # ---------------------------------------------------------
  print("\n[4] Map Input")
  data_map = {'key1': 1 , 'key2': 2}
  tm_map = TM(data_map)
  print(f"TM from Map keys: {tm_map.rn(2)}")
  
  print("\n--- Finished ---")

if __name__ == "__main__":
  CLI()

