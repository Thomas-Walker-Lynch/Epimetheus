#!/usr/bin/env python3
from collections import deque as FIFO

class SymbolSpace:
  """
  The manager of the Epimetheus integer namespace.
  """
  
  _counter = 0
  _dealloc_queue = FIFO()
  
  # HIERARCHY SUPPORT (Added)
  _parents = {} # Map: Child_Sym -> Parent_Sym

  class Instance:
    """The handle for a symbol."""
    __slots__ = ('_value' ,)

    def __init__(self ,value):
      self._value = value

    def __eq__(self ,other):
      # Compare value, not identity
      if isinstance(other ,SymbolSpace.Instance): return self._value == other._value
      return False

    def __hash__(self):
      return hash(self._value)

    def __repr__(self):
      return f"<Sym {self._value}>"

  @classmethod
  def alloc(cls) -> 'SymbolSpace.Instance':
    val = 0
    if cls._dealloc_queue:
      val = cls._dealloc_queue.popleft()
    else:
      cls._counter += 1
      val = cls._counter
    return cls.Instance(val)

  @classmethod
  def dealloc(cls ,sym: 'SymbolSpace.Instance'):
    val = sym._value
    if val == 0: raise ValueError("Null symbol (0) cannot be deallocated.")

    if val == cls._counter:
      cls._counter -= 1
      while cls._counter > 0 and cls._counter in cls._dealloc_queue:
        cls._dealloc_queue.remove(cls._counter)
        cls._counter -= 1
    else:
      cls._dealloc_queue.append(val)

  @classmethod
  def get_null(cls) -> 'SymbolSpace.Instance':
    return cls.Instance(0)

  # --- Hierarchy Methods ---

  @classmethod
  def set_parent(cls ,child ,parent):
    cls._parents[child] = parent

  @classmethod
  def get_parent(cls ,child):
    return cls._parents.get(child)

def verify():
  print(f"Allocating 3 symbols...")
  s1 = SymbolSpace.alloc()
  s2 = SymbolSpace.alloc()
  
  # Test Hierarchy
  SymbolSpace.set_parent(s2, s1)
  assert SymbolSpace.get_parent(s2) == s1
  print("Hierarchy check passed.")

def CLI():
  verify()

if __name__ == "__main__":
  CLI()
