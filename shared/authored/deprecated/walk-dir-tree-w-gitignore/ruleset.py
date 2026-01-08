# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-

"""
ruleset.py — layered Git ignore rule-set stack.

Implements the Git semantics:
  - Each directory can contribute patterns from .gitignore
  - Parent directories apply first
  - Last matching pattern wins
  - Negation overrides earlier ignores
  - dir-only rules respected
"""

from __future__ import annotations
import os
from typing import List, Optional
from .pattern import Pattern


class RuleSet:
  """
  Manages a stack of patterns from:
    - global excludes
    - .git/info/exclude
    - directory-local .gitignore

  push(patterns)
  pop(count)
  evaluate(path, is_dir)
  """

  def __init__(self) -> None:
    self.stack: List[List[Pattern]] = []

  def push(self, patterns: List[Pattern]) -> None:
    self.stack.append(patterns)

  def pop(self) -> None:
    if self.stack:
      self.stack.pop()

  def evaluate(self, relpath: str, is_dir: bool) -> bool:
    """
    Return True iff path is ignored.
    Last matching rule wins.
    """
    verdict: Optional[bool] = None

    for group in self.stack:
      for pat in group:
        if pat.matches(relpath, is_dir):
          if pat.negated:
            verdict = False
          else:
            verdict = True

    return bool(verdict)
