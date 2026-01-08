#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-

from __future__ import annotations

import os
import sys

# ----------------------------------------------------------------------
# Bootstrap import context when executed via symlink (e.g. ../walk)
# ----------------------------------------------------------------------
if __name__ == "__main__" and __package__ is None:
    # Resolve the real file (follows symlinks)
    _real = os.path.realpath(__file__)
    _pkg_dir = os.path.dirname(_real)
    _pkg_root = os.path.dirname(_pkg_dir)  # authored/

    # Ensure authored/ is on sys.path
    if _pkg_root not in sys.path:
        sys.path.insert(0, _pkg_root)

    # Force package name so relative imports work
    __package__ = "gitignore_treewalk"

# Now safe to do relative imports
from .pattern import Pattern
from .ruleset import RuleSet
from .walker import Walker
from .printer import Printer


# ----------------------------------------------------------------------
# Usage text
# ----------------------------------------------------------------------
def usage() -> int:
  print(
    "Usage:\n"
    "  walk |usage|help\n"
    "      Show this help.\n"
    "\n"
    "  walk list\n"
    "      Walk the working directory applying gitignore rules.\n"
  )
  return 0


# ----------------------------------------------------------------------
# CLI dispatcher
# ----------------------------------------------------------------------
def CLI(argv: List[str]) -> int:
  if not argv:
    return usage()

  cmd = argv[0]

  if cmd in ("usage", "help"):
    return usage()

  if cmd == "list":
    cwd = os.getcwd()
    cwd_dpa = os.path.abspath(cwd)

    rs = RuleSet.from_gitignore_files(
      start_dir=cwd_dpa
    )

    walker = Walker(
      root=cwd_dpa
      ,rules=rs
    )

    for p in walker.walk():
      print_path(
        p
        ,cwd_dpa
      )
    return 0

  print(f"Unknown command: {cmd}")
  return usage()


# ----------------------------------------------------------------------
# Entrypoint
# ----------------------------------------------------------------------
if __name__ == "__main__":
  sys.exit(
    CLI(
      sys.argv[1:]
    )
  )
