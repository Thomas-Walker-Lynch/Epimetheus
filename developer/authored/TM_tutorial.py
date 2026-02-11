#!/usr/bin/env python3
import TM
import sys

def print_tape(tm):
  """
  Prints tape contents using the First/Rest pattern.
  
  WARNING: This function moves the head!
  It rewinds to the start (LsR) and traverses to the end.
  To preserve head state on ND machines, pass 'tm.e()'.
  """
  # Reset head to start (Leftmost)
  tm.LsR()
  
  # Handle single element case
  if tm.qR() and tm.LqR():
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
  tm = TM.TM_Arr_CR_ND(data)
  
  while True:
    val = tm.r()
    print(f"Processing: {val}")
    if tm.qR(): break
    tm.s()

  print("\n--- 2. The Accumulator (Initialized First) ---")
  tm = TM.TM_Arr_CR_ND([1, 2, 3, 4, 5])
  
  total = tm.r()      # Process First
  while not tm.qR():  # Guard
    tm.s()          # Step
    total += tm.r() # Process Next
      
  print(f"Total: {total}")

  print("\n--- 3. Destructive Filter (Variable Tape) ---")
  # We use TM_ArrV_CLR_SO (Chiral Left Right, Solitary).
  # We need CLR to support 'eLsd' (Delete Left Neighbor).
  original_data = [1, 2, 3, 4, 5, 6, 7]
  tm = TM.TM_ArrV_CLR_SO(original_data)
  
  # 1. Process First (Index 0) - Special Case
  # We check the start. If odd, we must delete it.
  # Since we are at the start, we cannot use eLsd (no left neighbor).
  # We must Step then eLsd? No, that deletes 0.
  # If we Step, 0 becomes the left neighbor.
  
  tm.LsR() # Ensure start
  
  first_val = tm.r()
  if first_val % 2 != 0:
      print(f"Deleting odd start: {first_val}")
      if tm.qR(): 
          # Single element list. Cannot step.
          print("(Cannot delete last remaining element)")
      else:
          tm.s()    # Step to 1
          tm.eLsd() # Delete Left Neighbor (Index 0)
          # Head is now at Index 0 (the element that shifted down).
          # We must NOT step in the loop immediately, or we skip the new First.
          # To handle this cleanly in a loop, we align the head to "Previous" logic?
          # Easier: Just restart logic from the new current.
          
          # Since we deleted 0, the loop below starts at the *new* 0.
          # We just need to make sure we don't double step.
          # The simplest way is to handle the deletion, then let the loop 
          # run normally from the current head position.
  
  # Main Loop
  # Invariant: Head is on a valid cell we want to inspect.
  while True:
      val = tm.r()
      if val % 2 != 0:
          # Found odd. Delete it.
          print(f"Deleting odd: {val}")
          
          if not tm.qR():
              # Case: Not at end.
              # Step Right, then delete the item to our Left (which is 'val').
              tm.s()     
              tm.eLsd()  
              # Head is now on the next item (shifted left into current slot).
              # We continue loop to inspect *this* item.
          else:
              # Case: At End (Tail).
              # Cannot use eLsd strategy because we can't step right.
              # Must use esd strategy: Step Left, delete Right Neighbor.
              if tm.LqR():
                  print("(Last item, cannot empty tape)")
                  break
              
              tm.Ls()  # Step Left
              tm.esd() # Delete Right Neighbor (Tail)
              # Tail deleted. We are at new tail.
              # We inspected this even number already (presumably), so we break?
              # Actually, if we stepped left, we are on a number we already kept.
              break
      else:
          print(f"Keeping even: {val}")
          if tm.qR(): break
          tm.s()

  print("Final Tape Content:")
  # Pass the SO machine directly. 
  # This will consume/rewind the head, but that is acceptable here.
  print_tape(tm) 

if __name__ == "__main__":
  tm_loops_tutorial()
