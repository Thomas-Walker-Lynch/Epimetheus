#!/usr/bin/env python3
from TM import TMS, Features

def test_ar_machine():
  print("--- TM_ARRAY_SR_ND_aR Verification ---")

  # 1. Initialization with Feature (Mandatory Data)
  data = [10, 20, 30]
  t = TMS(data, features=[Features.APPEND_RIGHT])
  
  print(f"Created Machine t1. Len: {t.len()}")

  # 2. Test Append on t1
  t.s() # Step to 20
  print("Executing t1.aR(99)...")
  t.aR(99)
  print(f"t1 Len: {t.len()} (Expected: 4)")
  print(f"t1 Content from Head: {t.rn(3)} (Expected: [20, 30, 99])")

  # 3. Test Entanglement (Shared Tape)
  # ---------------------------------------------------------
  t2 = t.e()
  print("\nCreated entangled machine t2 from t1.")
  
  print("Executing t2.aR(100)...")
  t2.aR(100)
  
  # Verify t2 (The actor)
  print(f"t2 Len: {t2.len()} (Expected: 5)")
  
  # Verify t1 (The observer) - PROOF OF ENTANGLEMENT
  # t1 should see the change immediately because the tape is shared.
  print(f"t1 Len: {t.len()} (Expected: 5)")
  
  if t.len() == 5:
    print(">> SUCCESS: t1 reflects change made by t2 (Tape is shared).")
  else:
    print("!! FAIL: t1 did not see the change (Tape is copied?).")

  # 4. Verify Base Restrictions
  try:
    t.ls()
    print("!! FAIL: ls() succeeded")
  except AttributeError:
    print(">> SUCCESS: ls() blocked.")

if __name__ == "__main__":
  test_ar_machine()
