#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-


"""
ObjectRegistry

Maps Python runtime objects to ProcessLocalId using weak identity.

Constraints:
  - Only weakref-able Python objects can be registered.
  - This is intentional: RT properties attach to identity-bearing runtime instances,
    not to value-like primitives (ints/strings/lists/dicts).
"""

from __future__ import annotations

import weakref
from typing import Any ,Callable ,Dict ,Optional

from .ProcessLocalId import ProcessLocalIdGenerator ,ProcessLocalId


class ObjectRegistry:
  def __init__(self ,id_gen: ProcessLocalIdGenerator):
    self._id_gen = id_gen
    self._obj_to_id_wkd: "weakref.WeakKeyDictionary[Any ,ProcessLocalId]" = weakref.WeakKeyDictionary()
    self._id_to_obj_ref: Dict[ProcessLocalId ,weakref.ref] = {}
    self._finalizers: Dict[ProcessLocalId ,Callable[[ProcessLocalId],None]] = {}

  def register_finalizer(self ,fn: Callable[[ProcessLocalId],None]):
    """
    Registers a finalizer callback invoked when any registered object is GC'd.
    """
    self._global_finalizer = fn

  def _on_collect(self ,obj_id: ProcessLocalId):
    self._obj_to_id_wkd.pop(self._id_to_obj_ref[obj_id]() ,None)
    self._id_to_obj_ref.pop(obj_id ,None)
    fn = getattr(self ,"_global_finalizer" ,None)
    if fn is not None: fn(obj_id)

  def get_id(self ,obj: Any) -> ProcessLocalId:
    """
    Returns the ProcessLocalId for `obj`, registering it if needed.

    Raises TypeError if `obj` is not weakref-able.
    """
    try:
      existing = self._obj_to_id_wkd.get(obj)
    except TypeError:
      raise TypeError("ObjectRegistry: object is not weakref-able; RT properties do not attach to value-like primitives.")
    if existing is not None: return existing

    obj_id = self._id_gen.next_id()
    try:
      self._obj_to_id_wkd[obj] = obj_id
    except TypeError:
      raise TypeError("ObjectRegistry: object is not weakref-able; RT properties do not attach to value-like primitives.")
    self._id_to_obj_ref[obj_id] = weakref.ref(obj ,lambda _ref ,oid=obj_id: self._on_collect(oid))
    return obj_id

  def try_get_object(self ,obj_id: ProcessLocalId) -> Optional[Any]:
    """
    Best-effort: returns the live object, or None if it has been collected or never registered.
    """
    ref = self._id_to_obj_ref.get(obj_id)
    if ref is None: return None
    return ref()
