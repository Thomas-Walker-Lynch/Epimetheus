#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-

import sys
# CHANGED: Import directly from the package 'Epimetheus'
from Epimetheus import PropertyManager

def test_everything() -> int:
  """
  The Work Function.
  Returns 0 on success ,non-zero on failure.
  """
  try:
    pm = PropertyManager()
    
    # 1. Declare Properties (The Schema)
    prop_color = pm.declare_property("color")
    prop_meaning = pm.declare_property("meaning")

    # 2. Annotate an Entity (Instance)
    class Robot: pass
    r1 = Robot()
    
    pm.set(r1 ,"color" ,"silver")
    print(f"Robot Color: {pm.get(r1 ,'color')}") 
    # Expect: silver

    # 3. Annotate an Entity (Class)
    pm.set(Robot ,"meaning" ,"A machine")
    print(f"Robot Class Meaning: {pm.get(Robot ,'meaning')}")
    # Expect: A machine

    # 4. Annotate a Value (Primitive)
    pm.set(42 ,"meaning" ,"The Answer")
    print(f"42 Meaning: {pm.get(42 ,'meaning')}")
    # Expect: The Answer

    # Annotating a Tuple
    my_key = (1 ,2)
    pm.set(my_key ,"color" ,"invisible")
    print(f"Tuple Color: {pm.get((1 ,2) ,'color')}")
    # Expect: invisible

    print("\n--- Success ---")
    return 0

  except Exception as e:
    print(f"Test Failed: {e}")
    return 1


def CLI(args_seq) -> int:
  """
  The CLI Entry Point.
  """
  return test_everything()


if(__name__ == "__main__"):
  sys.exit(CLI(sys.argv))
