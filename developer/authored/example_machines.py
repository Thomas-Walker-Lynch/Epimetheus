#!/usr/bin/env python3
import TM as tm_lib

def example_machine_grammar():
  print("--- TM Machine Grammar Example ---")
  
  data = [10, 20, 30, 40, 50]

  # 1. The Default Machine (TM_Arr_SR_ND)
  t_def = tm_lib.TM(data)
  print(f"\n[Default] {type(t_def).__name__}")
  print(f"  Read: {t_def.r()}")
  t_def.s()
  print(f"  Step, Read: {t_def.r()}")
  
  # 2. The Mirror Machine (TM_Arr_SL_ND)
  t_mir = tm_lib.TM_SL(data)
  print(f"\n[Mirror] {type(t_mir).__name__}")
  t_mir.sR()
  print(f"  Cue Rightmost, Read: {t_mir.r()} (Exp: 50)")
  t_mir.ls()
  print(f"  Left Step, Read: {t_mir.r()} (Exp: 40)")
  
  # 3. Abstract Natural Number Machine (TMA)
  t_nat = tm_lib.TMA_NaturalNumber()
  print(f"\n[Abstract] {type(t_nat).__name__}")
  print(f"  Read: {t_nat.r()} (Exp: 0)")
  t_nat.sn(1000)
  print(f"  Step 1000, Read: {t_nat.r()} (Exp: 1000)")

  # 4. Explicit Grammar Construction
  # TM_ASCII_SR_SO (ASCII, Step Right, Solitary)
  # Uses the Array backing but with ASCII type name
  ascii_data = [65, 66, 67] # A, B, C
  t_asc = tm_lib.TM_ASCII_SR_SO(ascii_data)
  print(f"\n[Grammar Explicit] {type(t_asc).__name__}")
  print(f"  Read: {t_asc.r()} (Exp: 65)")

if __name__ == "__main__":
  example_machine_grammar()
