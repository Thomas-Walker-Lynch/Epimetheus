#!/usr/bin/env python3
"""
Symbol.py
Epimetheus Symbol Implementation.
Core Concept: A Symbol is a discrete, hashable set defined by its identity.
"""

import weakref

class Symbol(set):
    """
    A persistent, unique identity that acts as a container.
    
    Properties:
      - Identity: Equality and Hashing are based on memory address (id).
      - Container: Inherits from set. Can contain other symbols (differentiation).
      - Hierarchy: Knows its parent to support dealloc().
    """
    
    def __init__(self, parent=None):
        super().__init__()
        # We hold a strong ref to the parent? 
        # If we hold strong, the root keeps the whole tree alive.
        # If we hold weak, we can't dealloc if parent dies (but that's expected).
        # Standard graph theory: Edges are usually strong.
        self._parent = parent

    def alloc(self):
        """
        Allocates a new differentiated symbol from this one.
        The new symbol is added to this symbol (the set).
        """
        child = Symbol(parent=self)
        self.add(child)
        return child

    def dealloc(self):
        """
        Removes this symbol from its parent.
        If no other references exist, this symbol will be garbage collected.
        """
        if self._parent is not None:
            self._parent.remove(self)
            self._parent = None
        # If parent is None (Root), dealloc does nothing.

    # --- Identity Overrides (Force Set to be Hashable) ---

    def __hash__(self):
        return id(self)

    def __eq__(self, other):
        return self is other

    def __repr__(self):
        return f"<Symbol {id(self):x}>"

# --- Example ---

def example_symbol_lifecycle():
    print("--- Symbol Lifecycle Example ---")
    root = Symbol()

    # 1. Root exists
    print(f"Root: {root}")
    
    # 2. Alloc Features
    nav = root.alloc()
    print(f"Allocated 'nav' inside root: {nav}")
    print(f"root has nav? {nav in root}")
    
    # 3. Alloc Leaf
    left = nav.alloc()
    print(f"Allocated 'left' inside nav: {left}")
    
    # 4. Dealloc
    print("Deallocating 'left'...")
    left.dealloc()
    print(f"nav has left? {left in nav} (Expected: False)")

if __name__ == "__main__":
    example_symbol_lifecycle()
