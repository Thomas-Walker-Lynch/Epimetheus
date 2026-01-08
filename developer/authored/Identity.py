#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-


"""
Identity

An abstract identity used as the subject key for property attachment.

Kinds (strings) determine storage and resolution behavior.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any ,Optional ,Tuple

from .ProcessLocalId import ProcessLocalId


IDENTITY_KIND_PY_OBJECT = "py_object"
IDENTITY_KIND_SYNTAX = "syntax"
IDENTITY_KIND_PROPERTY = "property"
IDENTITY_KIND_SET = "semantic_set"


@dataclass(frozen=True ,slots=True)
class Identity:
  """
  `id` is always a ProcessLocalId.

  `kind` partitions lookup behavior.

  `payload` is kind-specific metadata (kept small; do not put giant graphs here).
  """
  id: ProcessLocalId
  kind: str
  payload: Any = None

  def __repr__(self) -> str:
    # Do not reveal id token.
    return f"<Identity kind={self.kind!r}>"
