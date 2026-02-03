#!/usr/bin/env python3
from SymbolSpace import SymbolSpace

# ==========================================
# THE GRAPH (Discrete Function)
# ==========================================

class DiscreteFunction:
  """
  The Knowledge Store.
  Supports 'Smart Postings' to Parent Symbols.
  """
  def __init__(self):
    self._rev = {} # reverse map: Symbol -> Set of Objects

  def _add_posting(self ,sym ,obj):
    if sym not in self._rev: self._rev[sym] = set()
    self._rev[sym].add(obj)

  def set(self ,obj_sym ,prop_sym):
    """
    Assigns a property. Updates indexes for the specific property
    AND its namespace (Parent).
    """
    # 1. Index Specific (e.g., #105 'Red')
    self._add_posting(prop_sym ,obj_sym)

    # 2. Index General (e.g., #50 'Color')
    parent = SymbolSpace.get_parent(prop_sym)
    if parent:
      self._add_posting(parent ,obj_sym)

  def find(self ,sym):
    """Returns the set of objects associated with this symbol."""
    return self._rev.get(sym ,set())

# --- Work Function ---

def verify_graph():
  print("--- DiscreteFunction Verification ---")
  # 1. Create Symbols
  sym_obj = SymbolSpace.alloc()
  sym_prop = SymbolSpace.alloc()
  sym_parent = SymbolSpace.alloc()
  
  # 2. Setup Hierarchy
  SymbolSpace.set_parent(sym_prop, sym_parent)
  
  # 3. Set Fact
  df = DiscreteFunction()
  df.set(sym_obj, sym_prop)
  
  # 4. Check Smart Posting
  print(f"Finding specific property: {len(df.find(sym_prop))} (Expected 1)")
  print(f"Finding parent category:   {len(df.find(sym_parent))} (Expected 1)")

def CLI():
  verify_graph()

if __name__ == "__main__":
  CLI()
