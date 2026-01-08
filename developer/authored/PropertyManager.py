#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-


"""
PropertyManager

Core RT property system.

Key decisions vs the earlier `property_manager.py`:
  - do NOT key by `object_path()` strings (avoids collisions) fileciteturn3file4
  - runtime objects are keyed by weak identity (ProcessLocalId assigned by ObjectRegistry)
  - properties are first-class entities (Property has an id), so properties can have properties

This remains process-local and in-memory.
"""

from __future__ import annotations

from typing import Any ,Dict ,Iterable ,List ,Optional ,Tuple ,Union

from .ProcessLocalId import ProcessLocalIdGenerator ,ProcessLocalId
from .ObjectRegistry import ObjectRegistry
from .PropertyStore import PropertyStore
from .Property import Property
from .SemanticSets import SemanticSet ,SemanticSetStore
from .Syntax import SyntaxInstance


NamePathLike = Union[str ,List[str] ,Tuple[str ,...]]


class PropertyManager:
  def __init__(self):
    self._id_gen = ProcessLocalIdGenerator()
    self._obj_reg = ObjectRegistry(self._id_gen)
    self._store = PropertyStore()
    self._sets = SemanticSetStore()

    # Declare-by-name registry
    self._name_path_to_property: Dict[Tuple[str ,...],Property] = {}
    self._property_id_to_property: Dict[ProcessLocalId ,Property] = {}

    self._name_path_to_set: Dict[Tuple[str ,...],SemanticSet] = {}
    self._set_id_to_set: Dict[ProcessLocalId ,SemanticSet] = {}

    # Optional syntax instances (if user chooses to model them)
    self._syntax_id_to_instance: Dict[ProcessLocalId ,SyntaxInstance] = {}

    # Finalization cleanup
    self._obj_reg.register_finalizer(self._on_subject_finalized)

  def _on_subject_finalized(self ,subject_id: ProcessLocalId):
    self._store.remove_subject(subject_id)
    self._sets.remove_subject(subject_id)

  def _normalize_name_path(self ,name_path: NamePathLike) -> Tuple[str ,...]:
    if isinstance(name_path ,tuple): return name_path
    if isinstance(name_path ,list): return tuple(name_path)
    if isinstance(name_path ,str): return tuple(name_path.split("."))
    raise TypeError("name_path must be str ,list[str] ,or tuple[str ,...]")

  # -------------------------
  # Identity acquisition
  # -------------------------
  def id_of_py_object(self ,obj: Any) -> ProcessLocalId:
    return self._obj_reg.get_id(obj)

  def create_syntax_identity(self ,syntax: SyntaxInstance) -> ProcessLocalId:
    sid = self._id_gen.next_id()
    self._syntax_id_to_instance[sid] = syntax
    return sid

  def try_get_syntax(self ,syntax_id: ProcessLocalId) -> Optional[SyntaxInstance]:
    return self._syntax_id_to_instance.get(syntax_id)

  # -------------------------
  # Property declaration
  # -------------------------
  def declare_property(self ,name_path: NamePathLike ,doc: str = "") -> ProcessLocalId:
    np = self._normalize_name_path(name_path)
    existing = self._name_path_to_property.get(np)
    if existing is not None: return existing.id
    pid = self._id_gen.next_id()
    p = Property(pid ,np ,doc)
    self._name_path_to_property[np] = p
    self._property_id_to_property[pid] = p
    return pid

  def property_id(self ,name_path: NamePathLike) -> ProcessLocalId:
    np = self._normalize_name_path(name_path)
    p = self._name_path_to_property.get(np)
    if p is None: raise KeyError(f"Property not declared: {np!r}")
    return p.id

  def try_get_property(self ,prop_id: ProcessLocalId) -> Optional[Property]:
    return self._property_id_to_property.get(prop_id)

  # -------------------------
  # Semantic sets
  # -------------------------
  def declare_set(self ,name_path: NamePathLike ,doc: str = "") -> ProcessLocalId:
    np = self._normalize_name_path(name_path)
    existing = self._name_path_to_set.get(np)
    if existing is not None: return existing.id
    sid = self._id_gen.next_id()
    s = SemanticSet(sid ,np ,doc)
    self._name_path_to_set[np] = s
    self._set_id_to_set[sid] = s
    return sid

  def add_to_set(self ,subject: Any ,set_id: ProcessLocalId):
    subject_id = self._coerce_subject_id(subject)
    self._sets.add_member(set_id ,subject_id)

  def is_in_set(self ,subject: Any ,set_id: ProcessLocalId) -> bool:
    subject_id = self._coerce_subject_id(subject)
    return self._sets.has_member(set_id ,subject_id)

  def members(self ,set_id: ProcessLocalId) -> List[ProcessLocalId]:
    return list(self._sets.members(set_id))

  # -------------------------
  # Set/get properties
  # -------------------------
  def set(self ,subject: Any ,prop: Union[ProcessLocalId ,NamePathLike] ,value: Any):
    subject_id = self._coerce_subject_id(subject)
    prop_id = self._coerce_property_id(prop)
    self._store.set(subject_id ,prop_id ,value)

  def get(self ,subject: Any ,prop: Union[ProcessLocalId ,NamePathLike] ,default: Any = None) -> Any:
    subject_id = self._coerce_subject_id(subject)
    prop_id = self._coerce_property_id(prop)
    return self._store.get(subject_id ,prop_id ,default)

  def has(self ,subject: Any ,prop: Union[ProcessLocalId ,NamePathLike]) -> bool:
    subject_id = self._coerce_subject_id(subject)
    prop_id = self._coerce_property_id(prop)
    return self._store.has(subject_id ,prop_id)

  def subjects_with(self ,prop: Union[ProcessLocalId ,NamePathLike]) -> List[ProcessLocalId]:
    prop_id = self._coerce_property_id(prop)
    return list(self._store.subjects_with(prop_id))

  # -------------------------
  # Coercions
  # -------------------------
  def _coerce_subject_id(self ,subject: Any) -> ProcessLocalId:
    if isinstance(subject ,ProcessLocalId): return subject
    # For Python runtime objects, we require weakref-able instances.
    return self._obj_reg.get_id(subject)

  def _coerce_property_id(self ,prop: Union[ProcessLocalId ,NamePathLike]) -> ProcessLocalId:
    if isinstance(prop ,ProcessLocalId): return prop
    return self.property_id(prop)
