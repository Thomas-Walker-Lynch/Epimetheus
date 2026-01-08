#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-


"""
SemanticSets

Membership sets over identities. Used for semantic typing.

Design:
  - set_id identifies the set
  - members are subject ids
  - reverse index for cleanup
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Dict ,Optional ,Set

from .ProcessLocalId import ProcessLocalId


@dataclass(frozen=True ,slots=True)
class SemanticSet:
  id: ProcessLocalId
  name_path: tuple[str ,...]
  doc: str = ""

  def __repr__(self) -> str:
    return f"<SemanticSet {'.'.join(self.name_path)!r}>"


class SemanticSetStore:
  def __init__(self):
    self._members: Dict[ProcessLocalId ,Set[ProcessLocalId]] = {}
    self._subject_to_sets: Dict[ProcessLocalId ,Set[ProcessLocalId]] = {}

  def add_member(self ,set_id: ProcessLocalId ,subject_id: ProcessLocalId):
    self._members.setdefault(set_id ,set()).add(subject_id)
    self._subject_to_sets.setdefault(subject_id ,set()).add(set_id)

  def has_member(self ,set_id: ProcessLocalId ,subject_id: ProcessLocalId) -> bool:
    return subject_id in self._members.get(set_id ,set())

  def members(self ,set_id: ProcessLocalId) -> Set[ProcessLocalId]:
    return set(self._members.get(set_id ,set()))

  def remove_subject(self ,subject_id: ProcessLocalId):
    set_ids = self._subject_to_sets.pop(subject_id ,set())
    for set_id in set_ids:
      m = self._members.get(set_id)
      if m is not None:
        m.discard(subject_id)
        if not m: self._members.pop(set_id ,None)
