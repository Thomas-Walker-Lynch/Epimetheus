# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-

"""
printer.py — utilities for printing path listings:
  - linear list
  - ASCII "tree" view where each line begins with the actual path,
    then optional visual decoration for humans.
"""

from __future__ import annotations
from pathlib import Path
from typing import Iterable


class Printer:
  @staticmethod
  def print_linear(paths: Iterable[Path], cwd: Path) -> None:
    for p in paths:
      rel = p.relative_to(cwd)
      print(rel.as_posix())

  @staticmethod
  def print_tree(paths: Iterable[Path], cwd: Path) -> None:
    """
    Print each line as:
      <relpath>    <drawing>

    Where <drawing> is ASCII tree structure.
    """
    items = sorted(paths, key=lambda p: p.relative_to(cwd).as_posix())
    rels = [p.relative_to(cwd).as_posix() for p in items]

    # Build a tree prefix for human reading
    for rel in rels:
      parts = rel.split("/")
      indent = "  " * (len(parts) - 1)
      branch = "└─ " if len(parts) > 1 else ""
      print(f"{rel}    {indent}{branch}")
