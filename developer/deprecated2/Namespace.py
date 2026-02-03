#!/usr/bin/env python3
from SymbolSpace import SymbolSpace
from DiscreteFunction import DiscreteFunction
from Binder import Binder
import bisect

# --- Base Factory ---
class DifferentiatedSymbol:
  def __init__(self, name):
    self.root_symbol = SymbolSpace.alloc()
    self._intern_map = {} 
    
  def __call__(self, value):
    if value not in self._intern_map:
      sym = SymbolSpace.alloc()
      self._intern_map[value] = sym
      SymbolSpace.set_parent(sym, self.root_symbol)
      self.on_new_symbol(value, sym) # Hook for subclasses
    return self._intern_map[value]
    
  def on_new_symbol(self, value, sym): pass

# --- The "Interval Tree" Upgrade ---
class OrderedNamespace(DifferentiatedSymbol):
  """
  A Namespace that maintains a Sorted Index for Range Queries.
  """
  def __init__(self, name):
    super().__init__(name)
    # Stores tuples of (Value, Symbol), sorted by Value
    self._sorted_index = []
    
  def on_new_symbol(self, value, sym):
    # Maintain sorted order (O(N) insertion, but fast reads)
    # In a real DB, this would be a B-Tree insert (O(log N))
    bisect.insort(self._sorted_index, (value, sym))
    
  def find_range(self, min_val, max_val):
    """
    Returns all symbols whose values fall in [min_val, max_val].
    Complexity: O(log N) to find bounds + O(K) to collect K matches.
    """
    # 1. Binary Search for the start point
    # We create a dummy tuple for comparison
    start_idx = bisect.bisect_left(self._sorted_index, (min_val, None))
    
    # 2. Binary Search for the end point
    # We use a dummy symbol that is "infinite" to ensure we catch duplicates of max_val
    # (or just rely on the tuple comparison logic)
    end_idx = bisect.bisect_right(self._sorted_index, (max_val, object()))
    
    # 3. Slice and Return Symbols
    results = []
    for i in range(start_idx, end_idx):
      val, sym = self._sorted_index[i]
      results.append(sym)
      
    return results

# --- Work Function ---

def verify_range_query():
  print("--- Range Query Verification ---")
  
  # 1. Setup
  Weight = OrderedNamespace("Weight")
  K = DiscreteFunction() # Knowledge Graph
  binder = Binder()
  
  # 2. Objects (Bikes with different weights)
  bike_light = object() # 50kg
  bike_med   = object() # 150kg
  bike_heavy = object() # 250kg
  
  sym_light = binder.get_symbol(bike_light)
  sym_med   = binder.get_symbol(bike_med)
  sym_heavy = binder.get_symbol(bike_heavy)
  
  # 3. Facts
  K.set(sym_light, Weight(50))
  K.set(sym_med,   Weight(150))
  K.set(sym_heavy, Weight(250))
  
  # 4. Range Query: 100kg to 200kg
  print("\nQuery: Find bikes between 100kg and 200kg")
  
  # Step A: Expansion (Ask Namespace)
  target_symbols = Weight.find_range(100, 200)
  print(f" -> Namespace found {len(target_symbols)} relevant symbols.")
  
  # Step B: Intersection (Ask Graph)
  found_objects = []
  for sym in target_symbols:
    # Use the Reverse Map
    objs = K.find(sym)
    found_objects.extend(objs)
    
  print(f" -> Graph found {len(found_objects)} objects.")
  
  # Verification
  # Should find only the medium bike
  assert sym_med in found_objects
  assert sym_light not in found_objects
  assert sym_heavy not in found_objects
  print(" -> SUCCESS: Only the 150kg bike was returned.")

def CLI():
  verify_range_query()

if __name__ == "__main__":
  CLI()
