#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-

"""
Symbol

The Definition of Identity.

Architecture:
  - Symbol: The namespace and manager (Factory).
  - Symbol.Instance: The atomic unit of identity (The Product).

Constraints:
  - Distinctness: Instances are unique.
  - Opaque: No internal state leakage.
  - Immutable Metadata: Name and Doc can be set exactly once.
  - Static Manager: The Symbol class cannot be instantiated.
"""

from __future__ import annotations

import weakref
from typing import Optional


class Symbol:
  """
  The Manager class.
  Maintains the global state for symbol metadata and token generation.
  """

  def __new__(cls):
    """
    Prevent instantiation. Symbol is a static namespace/factory.
    """
    raise TypeError("The Symbol class is a static namespace and cannot be instantiated.")

  # Global registry for metadata.
  # We use class-level storage so Instances remain lightweight (no back-ref needed).
  _names: weakref.WeakKeyDictionary[Symbol.Instance ,str] = weakref.WeakKeyDictionary()
  _docs: weakref.WeakKeyDictionary[Symbol.Instance ,str] = weakref.WeakKeyDictionary()

  # Inclusive bounding: Start at 0 (The Null Token)
  _current_token: int = 0

  class Instance:
    """
    The atomic unit of identity.
    """
    # __weakref__ required for WeakKeyDictionary keys
    __slots__ = (
      '_token'
      ,'__weakref__'
    )

    def __init__(self ,token: int):
      self._token = token

    def __repr__(self) -> str:
      return "<Symbol.Instance>"

    def __eq__(self ,other: object) -> bool:
      if( isinstance(other ,Symbol.Instance) ):
        return self._token == other._token
      return NotImplemented

    def __hash__(self) -> int:
      return hash(self._token)

    # ----------------------------------------------------------------------
    # Metadata Accessors
    # ----------------------------------------------------------------------

    @property
    def name(self) -> Optional[str]:
      """
      Returns the name of the symbol, or None if anonymous.
      """
      return Symbol._names.get(self)

    @name.setter
    def name(self ,value: str):
      """
      Sets the name.
      Raises RuntimeError if the name has already been set.
      """
      if( self in Symbol._names ):
        raise RuntimeError(f"Symbol name is immutable. Already set to '{Symbol._names[self]}'.")
      Symbol._names[self] = value

    @property
    def doc(self) -> str:
      """
      Returns the docstring of the symbol, or "" if none.
      """
      return Symbol._docs.get(self ,"")

    @doc.setter
    def doc(self ,value: str):
      """
      Sets the docstring.
      Raises RuntimeError if the docstring has already been set.
      Ignores empty strings (setting to "" is a no-op).
      """
      if( not value ): return

      if( self in Symbol._docs ):
        raise RuntimeError("Symbol docstring is immutable. Already set.")

      Symbol._docs[self] = value

  # ------------------------------------------------------------------------
  # Factory Methods
  # ------------------------------------------------------------------------

  @classmethod
  def make(cls ,name: Optional[str] = None ,doc: str = "") -> Instance:
    """
    Mints a new, distinct Original Symbol Instance.

    Args:
      name: Optional name. If provided, it becomes immutable.
      doc: Optional docstring. If provided (non-empty), it becomes immutable.
    """
    # The Rest: Increment first, so the first public symbol is 1.
    cls._current_token += 1
    token = cls._current_token

    instance = cls.Instance(token)

    if( name is not None ):
      # Direct injection to bypass the "check if set" logic of the setter
      cls._names[instance] = name

    if( doc ):
      cls._docs[instance] = doc

    return instance

  # ------------------------------------------------------------------------
  # Static Initialization (The First)
  # ------------------------------------------------------------------------
  # We perform this inside the class body using local names.
  # This creates the Null Symbol immediately upon class definition.

  # Note: We must use 'Instance' (local scope) not 'Symbol.Instance'
  # because 'Symbol' is not yet bound.
  null = Instance(0)
  _names[null] = "Null"
  _docs[null] = "The Null Symbol"
  

#  LocalWords:  Accessors
