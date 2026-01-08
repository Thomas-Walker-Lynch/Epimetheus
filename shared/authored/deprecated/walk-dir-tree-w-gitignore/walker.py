# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-

"""
walker.py — Git-aware directory traversal.

Features:
  - Loads global excludes
  - Loads .git/info/exclude if present
  - Loads .gitignore in each directory
  - Does NOT descend into ignored directories
  - Yields both files and directories (Path objects)
  - Always parent-before-child
  - Sorted lexicographically
"""

from __future__ import annotations
import os
from pathlib import Path
from typing import Iterator, List

from .pattern import Pattern
from .ruleset import RuleSet


class Walker:
  def __init__(self, root: Path) -> None:
    self.root = root.resolve()
    self.ruleset = RuleSet()

    # Load global and project-local excludes
    self._push_global_excludes()
    self._push_local_excludes()

  # ----------------------------------------------------------------------
  # Exclude Sources
  # ----------------------------------------------------------------------

  def _push_global_excludes(self) -> None:
    """
    Load user's global ignore file if present:
      ~/.config/git/ignore
      or ~/.gitignore_global
    """
    candidates = [
      Path(os.environ.get("XDG_CONFIG_HOME", Path.home() / ".config")) / "git" / "ignore",
      Path.home() / ".gitignore_global"
    ]
    patterns = []

    for f in candidates:
      if f.exists():
        for line in f.read_text().splitlines():
          p = Pattern.from_line(line)
          if p:
            patterns.append(p)
        break

    if patterns:
      self.ruleset.push(patterns)

  def _push_local_excludes(self) -> None:
    """
    Load <root>/.git/info/exclude
    """
    f = self.root / ".git" / "info" / "exclude"
    patterns = []
    if f.exists():
      for line in f.read_text().splitlines():
        p = Pattern.from_line(line)
        if p:
          patterns.append(p)

    if patterns:
      self.ruleset.push(patterns)

  # ----------------------------------------------------------------------
  # Walk
  # ----------------------------------------------------------------------

  def walk(self) -> Iterator[Path]:
    return self._walk_dir(self.root, prefix="")

  def _walk_dir(self, dpath: Path, prefix: str) -> Iterator[Path]:
    # Load .gitignore for this directory
    patterns = []
    gitignore = dpath / ".gitignore"
    if gitignore.exists():
      for line in gitignore.read_text().splitlines():
        p = Pattern.from_line(line)
        if p:
          patterns.append(p)

    self.ruleset.push(patterns)

    # Evaluate this directory (except root)
    if prefix:
      if self.ruleset.evaluate(prefix, is_dir=True):
        # ignored directories are NOT descended into
        self.ruleset.pop()
        return

    yield dpath

    # Enumerate children sorted
    entries: List[Path] = sorted(dpath.iterdir(), key=lambda p: p.name)

    for entry in entries:
      rel = entry.relative_to(self.root).as_posix()
      is_dir = entry.is_dir()

      # Skip ignored
      if self.ruleset.evaluate(rel, is_dir=is_dir):
        continue

      # Directories
      if is_dir:
        yield from self._walk_dir(entry, rel)
      else:
        yield entry

    self.ruleset.pop()
