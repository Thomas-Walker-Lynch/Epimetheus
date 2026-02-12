# Symbol.py
# RT format: 2-space indent, PascalCase type names, snake_case identifiers.

from __future__ import annotations

import threading
import weakref
from typing import Optional, Sequence, Tuple


class SymbolSpace:
  """
  A SymbolSpace provides:
  - A Null symbol (identity 0).
  - A ContextMap from symbol context keys to Factories.
  - Factories that issue symbols and reclaim identities on GC.
  """

  class Symbol:
    __slots__ = (
      "_identity"
      ,"_factory_ref"
      ,"_finalizer"
      ,"_factory_id"
      ,"__weakref__"
    )

    def __init__(self ,identity: int ,factory: Optional["SymbolSpace.Factory"]):
      self._identity = int(identity)

      if(self._identity == 0):
        self._factory_ref = None
        self._finalizer = None
        self._factory_id = 0
        return

      if(factory is None):
        raise ValueError("Non-null symbols require a factory.")

      self._factory_id = factory.factory_id
      self._factory_ref = weakref.ref(factory)

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

    def __repr__(self) -> str:
      if(self._identity == 0):
        return "Symbol(0)"
      return f"Symbol({self._identity} ,factory_id={self._factory_id})"

    def __hash__(self) -> int:
      return hash( (self._factory_id ,self._identity) )

    def __eq__(self ,other: object) -> bool:
      if( not isinstance(other ,SymbolSpace.Symbol) ):
        return False
      return (self._factory_id == other._factory_id) and (self._identity == other._identity)

  class Factory:
    """
    Issues identities from a monotonically increasing counter and a free set.

    Reclaim policy (from the spec):
    - When a reclaimed identity equals counter, decrement counter.
      Continue decrementing while counter is in the free set.
    - Otherwise, add the identity to the free set.
    """

    __slots__ = (
      "_lock"
      ,"_counter"
      ,"_free_set"
      ,"_space"
      ,"_context_key"
      ,"factory_id"
      ,"__weakref__"
    )

    def __init__(self ,space: "SymbolSpace" ,context_key: Tuple["SymbolSpace.Symbol" ,...] ,factory_id: int):
      self._lock = threading.RLock()
      self._counter = 0  # 0 is reserved for space.null
      self._free_set = set()  # Set[int]
      self._space = space
      self._context_key = context_key
      self.factory_id = int(factory_id)

    @property
    def counter(self) -> int:
      with self._lock:
        return self._counter

    @property
    def free_set_size(self) -> int:
      with self._lock:
        return len(self._free_set)

    def make(self) -> "SymbolSpace.Symbol":
      with self._lock:
        if(self._free_set):
          identity = min(self._free_set)
          self._free_set.remove(identity)
          return self._space.Symbol(identity ,self)

        self._counter += 1
        return self._space.Symbol(self._counter ,self)

    def _reclaim_identity(self ,identity: int) -> None:
      identity = int(identity)
      if(identity == 0):
        raise ValueError("Attempted to reclaim the null symbol identity 0.")

      with self._lock:
        if(identity > self._counter):
          raise ValueError(f"Attempted to reclaim identity {identity} above counter {self._counter}.")

        if(identity in self._free_set):
          raise ValueError(f"Identity {identity} already reclaimed.")

        if(identity == self._counter):
          self._counter -= 1
          while(self._counter in self._free_set):
            self._free_set.remove(self._counter)
            self._counter -= 1
          return

        self._free_set.add(identity)

  class ContextMap:
    """
    Map: context_key -> Factory
    A context_key is an ordered list of symbols, represented as a tuple.
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
      self._factory_map = {}  # Dict[Tuple[Symbol] ,Factory]
      self._factory_id_counter = 0

    def _normalize(self ,context_key: Optional[Sequence["SymbolSpace.Symbol"]]) -> Tuple["SymbolSpace.Symbol" ,...]:
      if(context_key is None):
        return tuple()
      if( isinstance(context_key ,tuple) ):
        ctx = context_key
      else:
        ctx = tuple(context_key)

      for s in ctx:
        if( not isinstance(s ,SymbolSpace.Symbol) ):
          raise TypeError("context_key entries must be SymbolSpace.Symbol values.")

      return ctx

    def read(self ,context_key: Optional[Sequence["SymbolSpace.Symbol"]] = None) -> Optional["SymbolSpace.Factory"]:
      ctx = self._normalize(context_key)
      with self._lock:
        return self._factory_map.get(ctx)

    def make(self ,context_key: Optional[Sequence["SymbolSpace.Symbol"]] = None) -> "SymbolSpace.Factory":
      ctx = self._normalize(context_key)
      with self._lock:
        if(ctx in self._factory_map):
          raise KeyError("Factory already exists for this context_key.")

        self._factory_id_counter += 1
        f = SymbolSpace.Factory(self._space ,ctx ,self._factory_id_counter)
        self._factory_map[ctx] = f
        return f

    def delete(self ,context_key: Optional[Sequence["SymbolSpace.Symbol"]] = None) -> None:
      ctx = self._normalize(context_key)
      with self._lock:
        if(ctx not in self._factory_map):
          raise KeyError("No factory exists for this context_key.")
        del self._factory_map[ctx]

    def __len__(self) -> int:
      with self._lock:
        return len(self._factory_map)

  def __init__(self):
    self.null = SymbolSpace.Symbol(0 ,None)
    self.context_map = SymbolSpace.ContextMap(self)

    # Default root factory for empty context.
    self.root_factory = self.context_map.make(tuple())
