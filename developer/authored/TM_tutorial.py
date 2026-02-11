#!/usr/bin/env python3
import TM
import sys

def print_tape(tm):
  """
  Prints tape contents using the First/Rest pattern and vertical comma format.
  """
  # Reset head to start
  tm.LsR()
  
  if tm.qR() and tm.qL():
    # Handle single element or empty case (qL returns true at start)
    # Check if we have at least one element by trying to read?
    # Actually qL and qR being true means 1 element or 0?
    # qL=True (at start). qR=True (at end). => 1 element.
    # We rely on 'r' to throw/work.
    try:
      print(tm.r())
    except:
      print("(empty)")
    return

  # FIRST
  sys.stdout.write(f"{tm.r()}")
  
  # REST
  while not tm.qR():
    tm.s()
    sys.stdout.write(f" ,{tm.r()}")
  print("")

def tm_loops_tutorial():
  print("--- 1. The Standard Traversal (Read -> Process -> Step) ---")
  data = [10, 20, 30, 40, 50]
  tm = TM.TM_Arr_SR_ND(data)
  
  while True:
    val = tm.r()
    print(f"Processing: {val}")
    if tm.qR(): break
    tm.s()

  print("\n--- 2. The Accumulator (Initialized First) ---")
  tm = TM.TM_Arr_SR_ND([1, 2, 3, 4, 5])
  
  total = tm.r()      # Process First
  while not tm.qR():  # Guard
    tm.s()          # Step
    total += tm.r() # Process Next
      
  print(f"Total: {total}")

  print("\n--- 3. Destructive Filter (Entangled Copy) ---")
  # We use TM_ArrV_SR_SO because we need 'd' (delete).
  # We added 'e' (entangle) to the SO table to support this specific workflow.
  original_data = [1, 2, 3, 4, 5, 6, 7]
  tm_master = TM.TM_ArrV_SR_SO(original_data)
  
  # Create an entangled copy for the deletion work
  tm_worker = tm_master.e()
  
  # Work on the worker
  while True:
    val = tm_worker.r()
    
    if val % 2 != 0:
      print(f"Deleting odd: {val}")
      # Unguarded delete: We must guarantee we don't delete the final cell
      # in a way that breaks the invariant for the *other* machine if it were
      # elsewhere. But here they are entangled.
      # Note: If we delete the last element, the C logic steps back.
      tm_worker.d()
      
      # Since d() is in-place, the 'next' element slides into the current spot.
      # We do NOT step. But we must check if we hit the end of the tape
      # (i.e. we deleted the tail and stepped back, or the tape is empty).
      
      # Safety check for end of tape after delete
      if tm_worker.qR():
        # We are at the right edge. Check if the current (last) element
        # also needs deleting.
        val = tm_worker.r()
        if val % 2 != 0:
           print(f"Deleting last odd: {val}")
           try:
             tm_worker.d()
           except RuntimeError:
             print("(Tape became empty)")
             break
        break
    else:
      print(f"Keeping even: {val}")
      if tm_worker.qR(): break
      tm_worker.s()

  print("Final Tape Content:")
  print_tape(tm_master)

if __name__ == "__main__":
  tm_loops_tutorial()
