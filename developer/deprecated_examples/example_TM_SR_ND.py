#!/usr/bin/env python3
from TM import TM, Features

def example_bidirectional_machine():
  print("--- TM·Array_ND (Bidirectional) Example ---")
  
  data = [10, 20, 30, 40, 50]
  
  # Request 'L' feature to get the ND machine
  tm = TM(data, [Features.L])
  print(f"Created TM with Feature 'L'. Data: {data}")

  # 1. Forward to Middle
  tm.sn(2)
  print(f"sn(2) -> Read: {tm.r()} (Exp: 30)")
  
  # 2. Left Step (ls) - Only available on ND
  print("\n[Left Step]")
  tm.ls()
  print(f"ls() -> Read: {tm.r()} (Exp: 20)")
  
  # 3. Query Left (qL)
  print("\n[Query Left]")
  print(f"At 20. qL(): {tm.qL()} (Exp: False)")
  tm.ls() # At 10 (Start)
  print(f"ls() -> At 10. qL(): {tm.qL()} (Exp: True)")

  # 4. Cue Rightmost (sR)
  print("\n[Cue Rightmost]")
  tm.sR()
  print(f"sR() -> Read: {tm.r()} (Exp: 50)")
  print(f"qR(): {tm.qR()} (Exp: True)")

if __name__ == "__main__":
  example_bidirectional_machine()
