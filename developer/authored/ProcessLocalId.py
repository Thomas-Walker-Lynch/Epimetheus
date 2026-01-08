#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-


"""
ProcessLocalId

A process-local identifier used as an internal key.

Design constraint:
  - NOT intended to be serialized or persisted.
  - `repr()` intentionally does not reveal the numeric token, to discourage logging/persistence.
"""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True ,slots=True)
class ProcessLocalId:
  _n: int

  def __repr__(self) -> str:
    return "<ProcessLocalId>"

  def __str__(self) -> str:
    return "<ProcessLocalId>"

  def as_int_UNSAFE(self) -> int:
    """
    Returns the raw integer token.

    UNSAFE because:
      - tokens are process-local
      - do not write these into files/databases/logs as stable identifiers
    """
    return self._n


class ProcessLocalIdGenerator:
  """
  Monotonic generator; ids are never recycled.
  """
  def __init__(self ,start: int = 1):
    if start < 1: raise ValueError("start must be >= 1")
    self._next_n: int = start

  def next_id(self) -> ProcessLocalId:
    n = self._next_n
    self._next_n += 1
    return ProcessLocalId(n)
