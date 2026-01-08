#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-


"""
Syntax

RT syntax identity instances.

We treat "syntax" as AST-level objects:
  - kind: official-ish AST node kind name (e.g., "ast.FunctionDef")
  - location: file + span
  - scope: enclosing syntax identity id (optional)
  - parts: mapping of part-name to literal or referenced syntax identity id(s)

This module does NOT traverse Python programs. It only defines the data model.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any ,Dict ,Optional ,Tuple ,Union ,List

from .ProcessLocalId import ProcessLocalId


@dataclass(frozen=True ,slots=True)
class SourceSpan:
  file_path: str
  lineno: int
  col: int
  end_lineno: int
  end_col: int


SyntaxPartValue = Union[
  None
  ,bool
  ,int
  ,float
  ,str
  ,ProcessLocalId
  ,List["SyntaxPartValue"]
  ,Dict[str ,"SyntaxPartValue"]
]


@dataclass(frozen=True ,slots=True)
class SyntaxInstance:
  """
  A single syntax node instance.

  NOTE: many syntax nodes have no identifier-name. Name-like things (identifiers)
  appear as child nodes or literals inside `parts`.
  """
  kind: str
  span: SourceSpan
  scope_id: Optional[ProcessLocalId] = None
  parts: Dict[str ,SyntaxPartValue] = None
