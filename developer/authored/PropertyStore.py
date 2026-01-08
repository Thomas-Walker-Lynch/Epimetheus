#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-


"""
PropertyStore

Stores property values and maintains reverse lookups.

This is intentionally process-local and in-memory.
"""

from __future__ import annotations

from typing import Any ,Dict ,Optional ,Set ,Tuple

from .ProcessLocalId import ProcessLocalId


class PropertyStore:
  def __init__(self):
    # (subject_id ,property_id) -> value
    self._values: Dict[Tuple[ProcessLocalId ,ProcessLocalId] ,Any] = {}

    # subject_id -> set(property_id)
    self._subject_to_props: Dict[ProcessLocalId ,Set[ProcessLocalId]] = {}

    # property_id -> set(subject_id)
    self._prop_to_subjects: Dict[ProcessLocalId ,Set[ProcessLocalId]] = {}

  def set(self ,subject_id: ProcessLocalId ,prop_id: ProcessLocalId ,value: Any):
    key = (subject_id ,prop_id)
    self._values[key] = value
    self._subject_to_props.setdefault(subject_id ,set()).add(prop_id)
    self._prop_to_subjects.setdefault(prop_id ,set()).add(subject_id)

  def get(self ,subject_id: ProcessLocalId ,prop_id: ProcessLocalId ,default: Any = None) -> Any:
    return self._values.get((subject_id ,prop_id) ,default)

  def has(self ,subject_id: ProcessLocalId ,prop_id: ProcessLocalId) -> bool:
    return (subject_id ,prop_id) in self._values

  def subjects_with(self ,prop_id: ProcessLocalId) -> Set[ProcessLocalId]:
    return set(self._prop_to_subjects.get(prop_id ,set()))

  def props_of(self ,subject_id: ProcessLocalId) -> Set[ProcessLocalId]:
    return set(self._subject_to_props.get(subject_id ,set()))

  def remove_subject(self ,subject_id: ProcessLocalId):
    """
    Remove all stored properties for a subject (used on finalization).
    """
    prop_ids = self._subject_to_props.pop(subject_id ,set())
    for prop_id in prop_ids:
      self._values.pop((subject_id ,prop_id) ,None)
      s = self._prop_to_subjects.get(prop_id)
      if s is not None:
        s.discard(subject_id)
        if not s: self._prop_to_subjects.pop(prop_id ,None)
