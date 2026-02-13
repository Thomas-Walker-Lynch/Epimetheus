# Symbol.py
# RT format: 2-space indent, PascalCase type names, snake_case identifiers.

from __future__ import annotations

import threading
import weakref
from enum import Enum, auto
from typing import Optional, Sequence, Tuple


class Symbol:
  __slots__ = (
    "_identity"
    ,"_factory_ref"
    ,"_finalizer"
    ,"_factory_id"
    ,"__weakref__"
  )

  def __init__(self ,identity: int ,factory: Optional["FactoryS"]):
    self._identity = int(identity)

    if(factory is None):
      self._factory_id = 0
      self._factory_ref = None
      self._finalizer = None
      return

    self._factory_id = factory.factory_id
    self._factory_ref = weakref.ref(factory)

    # Capture state for finalizer to avoid keeping strong ref to self
    identity_copy = self._identity
    factory_ref = self._factory_ref

    def _finalize():
      f = factory_ref()
      if(f is None):
        return
      try:
        f._reclaim_identity(identity_copy)
      except Exception:
        # Finalizers must never raise.
        return

    self._finalizer = weakref.finalize(self ,_finalize)

  @property
  def identity(self) -> int:
    return self._identity

  @property
  def factory(self) -> Optional["FactoryS"]:
    if(self._factory_ref is None):
      return None
    return self._factory_ref()

  def __repr__(self) -> str:
    return f"Symbol({self._identity} ,factory_id={self._factory_id})"

  def __hash__(self) -> int:
    return hash( (self._factory_id ,self._identity) )

  def __eq__(self ,other: object) -> bool:
    if( not isinstance(other ,Symbol) ):
      return False
    return (self._factory_id == other._factory_id) and (self._identity == other._identity)


class Factory:
  """
  Base First-Order Factory.
  - created with a known context.
  - strictly increases counter.
  - manages free set.
  """
  __slots__ = (
    "_lock"
    ,"_counter"
    ,"_free_set"
    ,"_space"
    ,"_context_key"
    ,"factory_id"
  )

  def __init__(self ,space: "SymbolSpace" ,context_key: Tuple["Symbol" ,...] ,factory_id: int):
    self._lock = threading.RLock()
    self._counter = 0
    self._free_set = set()
    self._space = space
    self._context_key = context_key
    self.factory_id = int(factory_id)

  def make(self ,wrapper: "FactoryS") -> "Symbol":
    with self._lock:
      if(self._free_set):
        identity = min(self._free_set)
        self._free_set.remove(identity)
        return Symbol(identity ,wrapper)

      identity = self._counter
      self._counter += 1
      return Symbol(identity ,wrapper)

  def has(self ,symbol: "Symbol") -> bool:
    # FactoryS handles the ID check; we handle logic.
    with self._lock:
      if(symbol.identity >= self._counter):
        return False
      if(symbol.identity in self._free_set):
        return False
    return True

  def reclaim(self ,identity: int) -> None:
    with self._lock:
      if(identity >= self._counter):
        raise ValueError(f"Identity {identity} not issued (counter {self._counter}).")
      
      if(identity in self._free_set):
        raise ValueError(f"Identity {identity} already reclaimed.")

      if(identity == self._counter - 1):
        self._counter -= 1
        while( (self._counter > 0) and ((self._counter - 1) in self._free_set) ):
          self._counter -= 1
          self._free_set.remove(self._counter)
        return

      self._free_set.add(identity)


class FactoryS:
  """
  Factory with Status (Second Order).
  Wraps the base Factory using VTable swapping.
  """
  __slots__ = (
    "_space"
    ,"_context_key"
    ,"_factory_id"
    ,"_base"
    # VTable slots
    ,"make"
    ,"has"
    ,"_reclaim_identity"
  )

  def __init__(self ,space: "SymbolSpace" ,context_key: Tuple["Symbol" ,...] ,factory_id: int):
    self._space = space
    self._context_key = context_key
    self._factory_id = factory_id
    self._base = None
    
    # Initialize in Empty state
    self._change_state_empty()

  @property
  def factory_id(self) -> int:
    return self._factory_id

  # --- State Transitions ---

  def _change_state_empty(self):
    self.make = self._make_empty
    self.has = self._has_empty
    self._reclaim_identity = self._reclaim_empty

  def _change_state_active(self ,base_factory: "Factory"):
    self._base = base_factory
    # Hot-swap to active methods
    self.make = self._make_active
    self.has = self._has_active
    self._reclaim_identity = self._reclaim_active

  # --- Empty State Methods ---

  def _make_empty(self) -> "Symbol":
    # JIT creation of the base factory
    base = Factory(self._space ,self._context_key ,self._factory_id)
    
    # Transition to Active
    self._change_state_active(base)
    
    # Delegate to the new base (which will issue identity 0)
    return base.make(self)

  def _has_empty(self ,symbol: "Symbol") -> bool:
    return False

  def _reclaim_empty(self ,identity: int) -> None:
    # Should not happen if logic is correct, as no symbols exist to be reclaimed.
    raise RuntimeError("Attempted to reclaim symbol from an Empty FactoryS.")

  # --- Active State Methods ---

  def _make_active(self) -> "Symbol":
    # Direct delegation
    return self._base.make(self)

  def _has_active(self ,symbol: "Symbol") -> bool:
    if(symbol._factory_id != self._factory_id):
      return False
    return self._base.has(symbol)

  def _reclaim_active(self ,identity: int) -> None:
    self._base.reclaim(identity)


class ContextMap:
  """
  Map: context_key -> FactoryS
  """
  __slots__ = (
    "_space"
    ,"_lock"
    ,"_factory_map"
    ,"_factory_id_counter"
  )

  def __init__(self ,space: "SymbolSpace"):
    self._space = space
    self._lock = threading.RLock()
    self._factory_map = {}
    self._factory_id_counter = 0

  def _normalize(self ,context_key: Optional[Sequence["Symbol"]]) -> Tuple["Symbol" ,...]:
    if(context_key is None):
      return tuple()
    if( isinstance(context_key ,tuple) ):
      ctx = context_key
    else:
      ctx = tuple(context_key)

    for s in ctx:
      if( not isinstance(s ,Symbol) ):
        raise TypeError("context_key entries must be Symbol values.")

    return ctx

  def read(self ,context_key: Optional[Sequence["Symbol"]] = None) -> Optional["FactoryS"]:
    ctx = self._normalize(context_key)
    with self._lock:
      return self._factory_map.get(ctx)

  def make(self ,context_key: Optional[Sequence["Symbol"]] = None) -> "FactoryS":
    ctx = self._normalize(context_key)
    with self._lock:
      if(ctx in self._factory_map):
        raise KeyError("FactoryS already exists for this context_key.")

      self._factory_id_counter += 1
      # Create FactoryS (starts Empty)
      f = FactoryS(self._space ,ctx ,self._factory_id_counter)
      self._factory_map[ctx] = f
      return f

  def delete(self ,context_key: Optional[Sequence["Symbol"]] = None) -> None:
    ctx = self._normalize(context_key)
    with self._lock:
      if(ctx not in self._factory_map):
        raise KeyError("No factory exists for this context_key.")
      del self._factory_map[ctx]

  def __len__(self) -> int:
    with self._lock:
      return len(self._factory_map)


class SymbolSpace:
  """
  Container for the ContextMap and Root FactoryS.
  """
  def __init__(self):
    self.context_map = ContextMap(self)
    
    # Create the root factory (empty context)
    # Returns a FactoryS (in Empty state)
    self.root_factory = self.context_map.make(tuple())
