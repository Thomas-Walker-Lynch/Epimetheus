# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-

"""
pattern.py — Git ignore pattern parser.

Implements:
  Git pattern semantics:
    - !negation
    - directory-only ('foo/')
    - anchored ('/foo')
    - wildcards '*', '?'
    - recursive wildcard '**'
    - full-path matching
    - last rule wins
"""

from __future__ import annotations
import os
import re
from dataclasses import dataclass
from typing import Optional


@dataclass
class Pattern:
  raw: str
  negated: bool
  anchored: bool
  dir_only: bool
  regex: re.Pattern

  @staticmethod
  def from_line(line: str) -> Optional["Pattern"]:
    """
    Parse a single .gitignore pattern line.
    Return None for comments/empty.
    """

    stripped = line.strip()
    if not stripped or stripped.startswith("#"):
      return None

    negated = stripped.startswith("!")
    if negated:
      stripped = stripped[1:].lstrip()
      if not stripped:
        return None

    dir_only = stripped.endswith("/")
    if dir_only:
      stripped = stripped[:-1]

    anchored = stripped.startswith("/")
    if anchored:
      stripped = stripped[1:]

    # Convert git-style pattern to regex
    # Git semantics:
    #   **  -> match any depth
    #   *   -> match any sequence except '/'
    #   ?   -> match one char except '/'
    #
    # Always match against full path (unix style, no leading '.')
    #
    def escape(s: str) -> str:
      return re.escape(s)

    # Convert pattern piecewise
    regex_pieces = []
    i = 0
    while i < len(stripped):
      c = stripped[i]
      if c == "*":
        # Check for **
        if i + 1 < len(stripped) and stripped[i + 1] == "*":
          # '**' -> match zero or more directories OR characters
          regex_pieces.append(".*")
          i += 2
        else:
          # '*' -> match any chars except '/'
          regex_pieces.append("[^/]*")
          i += 1
      elif c == "?":
        regex_pieces.append("[^/]")
        i += 1
      else:
        regex_pieces.append(escape(c))
        i += 1

    regex_string = "".join(regex_pieces)

    # Anchored: match from start of path
    # Unanchored: match anywhere in path
    if anchored:
      full = fr"^{regex_string}$"
    else:
      full = fr"(^|/){regex_string}($|/)"

    return Pattern(
      raw=line,
      negated=negated,
      anchored=anchored,
      dir_only=dir_only,
      regex=re.compile(full),
    )

  def matches(self, relpath: str, is_dir: bool) -> bool:
    """
    Match full relative path, not just basename.
    """
    # If pattern is directory-only, relpath must be a directory
    if self.dir_only and not is_dir:
      return False

    return bool(self.regex.search(relpath))
