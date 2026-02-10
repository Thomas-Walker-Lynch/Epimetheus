#!/usr/bin/env python3
from TM import TM

def example_differentiation():
  print("--- TM Factory Differentiation Example ---")
  
  data = [10, 20, 30]

  # 1. Default (No Features)
  t1 = TM(data)
  print(f"\nRequest: Default")
  print(f"Result Type: {type(t1).__name__} (Expected: TM_Array_SR_ND)")
  
  # 2. Mirror View (Feature L)
  # Use the underscore syntax
  feat_L = TM.feature_L
  
  t2 = TM(data, [feat_L])
  print(f"\nRequest: Feature L ({feat_L})")
  print(f"Result Type: {type(t2).__name__} (Expected: TM_Array_ND)")
  
  # 3. Explicit Defaults (Should be ignored/stripped)
  feat_SR = TM.feature_SR
  t3 = TM(data, [feat_SR, feat_L])
  print(f"\nRequest: Features SR + L")
  print(f"Result Type: {type(t3).__name__} (Expected: TM_Array_ND)")

if __name__ == "__main__":
  example_differentiation()
