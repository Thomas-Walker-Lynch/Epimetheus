#!/usr/bin/env python3
import weakref
# Assuming SymbolSpace is in the same package or path
from SymbolSpace import SymbolSpace

class Binder:
  """
  Maps transient Runtime Objects to persistent Epimetheus Symbols.
  """
  
  def __init__(self):
    # Weak Key: If the Object dies, this entry disappears automatically.
    # Value: The Symbol (Strong reference).
    self._obj_to_sym = weakref.WeakKeyDictionary()

  def get_symbol(self ,obj) -> SymbolSpace.Instance:
    """
    Returns the symbol for the object.
    If none exists, mints a new one and binds it.
    """
    # Check if we already know this object
    if obj in self._obj_to_sym:
      return self._obj_to_sym[obj]

    # Mint new identity
    sym = SymbolSpace.alloc()
    self._obj_to_sym[obj] = sym
    
    return sym

  def lookup(self ,obj) -> SymbolSpace.Instance:
    """
    Non-allocating lookup. Returns None if object is unknown.
    """
    return self._obj_to_sym.get(obj ,None)

# --- Work & CLI ---

def verify_binder():
  print("--- Binder Verification ---")
  binder = Binder()
  
  # 1. Create a transient object
  class Bike: pass
  my_bike = Bike()
  
  # 2. Bind it
  sym = binder.get_symbol(my_bike)
  print(f"Bike is bound to: {sym}")
  
  # 3. Verify stability
  sym2 = binder.get_symbol(my_bike)
  assert sym == sym2
  print("Symbol is stable for same object.")
  
  # 4. Garbage Collection Test
  import gc
  del my_bike
  gc.collect()
  
  # The binder should be empty now because the key is dead
  # Note: The 'sym' variable still holds the integer, 
  # so the Concept survives even though the Implementation is gone.
  print("Object deleted. Binder entry should be gone (internal check).")
  print(f"Binder size: {len(binder._obj_to_sym)}") 

def CLI():
  verify_binder()

if __name__ == "__main__":
  CLI()

