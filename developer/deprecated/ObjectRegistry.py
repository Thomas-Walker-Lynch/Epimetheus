#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-

"""
ObjectRegistry

Maps Python runtime objects to ProcessLocalId.

Strategies:
  1. Weak Identity (Preferred): For objects that support weakrefs (instances ,classes).
     - ID lifetime is bound to object lifetime.
     - Auto-cleaned on GC.
  
  2. Value Identity (Fallback): For immutable primitives (int ,str ,tuple).
     - ID is bound to the *value* (hash/equality).
     - Stored strongly (since values like 42 or "red" are conceptually eternal).
"""

from __future__ import annotations

import weakref
from typing import Any ,Callable ,Dict ,Optional

from .ProcessLocalId import ProcessLocalIdGenerator ,ProcessLocalId


class ObjectRegistry:
  def __init__(self ,id_gen: ProcessLocalIdGenerator):
    self._id_gen = id_gen
    
    # Strategy 1: Entities (Weakref-able)
    self._obj_to_id_wkd: "weakref.WeakKeyDictionary[Any ,ProcessLocalId]" = weakref.WeakKeyDictionary()
    self._id_to_obj_ref: Dict[ProcessLocalId ,weakref.ref] = {}
    
    # Strategy 2: Values (Hashable ,not weakref-able)
    self._value_to_id: Dict[Any ,ProcessLocalId] = {}
    
    self._finalizers: Dict[ProcessLocalId ,Callable[[ProcessLocalId] ,None]] = {}
    self._global_finalizer: Optional[Callable[[ProcessLocalId] ,None]] = None

  def register_finalizer(self ,fn: Callable[[ProcessLocalId] ,None]):
    """
    Registers a finalizer callback invoked when any registered *weak-refable* object is GC'd.
    """
    self._global_finalizer = fn

  def _on_collect(self ,obj_id: ProcessLocalId):
    # Only called for Strategy 1 objects
    ref = self._id_to_obj_ref.pop(obj_id ,None)
    if(ref is not None):
      # The WeakKeyDictionary auto-cleans the forward mapping ,
      # but we double check or clean any edge cases if needed.
      pass 
      
    fn = self._global_finalizer
    if(fn is not None):
      fn(obj_id)

  def get_id(self ,obj: Any) -> ProcessLocalId:
    """
    Returns the ProcessLocalId for `obj` ,registering it if needed.
    """
    # 1. Try WeakRef Strategy (Entities)
    try:
      existing = self._obj_to_id_wkd.get(obj)
      if(existing is not None):
        return existing
    except TypeError:
      # obj is not weakref-able (e.g. int ,str ,tuple ,or list).
      # Fall through to Strategy 2.
      pass
    else:
      # It IS weakref-able ,but wasn't in the dictionary. Register it.
      obj_id = self._id_gen.next_id()
      self._obj_to_id_wkd[obj] = obj_id
      # Create reverse lookup with callback
      self._id_to_obj_ref[obj_id] = weakref.ref(obj ,lambda _ref ,oid=obj_id: self._on_collect(oid))
      return obj_id

    # 2. Try Value Strategy (Primitives)
    # Note: Mutable non-weakrefables (like standard lists) will fail here because they are unhashable.
    try:
      existing = self._value_to_id.get(obj)
      if(existing is not None):
        return existing
      
      # Register new value
      obj_id = self._id_gen.next_id()
      self._value_to_id[obj] = obj_id
      return obj_id
    except TypeError:
      # It is neither weakref-able NOR hashable (e.g. standard list ,dict).
      raise TypeError(
        f"ObjectRegistry: cannot track object of type {type(obj)!r}. "
        "It is neither weakref-able (Entity) nor hashable (Value)."
      )

  def try_get_object(self ,obj_id: ProcessLocalId) -> Optional[Any]:
    """
    Best-effort: returns the live object/value.
    """
    # Check Entities
    ref = self._id_to_obj_ref.get(obj_id)
    if(ref is not None):
      return ref()
      
    # Check Values
    # For now ,we assume values identify themselves.
    for val ,pid in self._value_to_id.items():
      if(pid == obj_id):
        return val
        
    return None
