#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-

"""
meta.py - thin wrappers around command modules

Current responsibilities:
  1. Load the incommon 'printenv' command module (no .py extension)
     using load_command_module.load_command_module().
  2. Expose printenv() here, calling the imported printenv() work
     function with default arguments (equivalent to running without
     any CLI arguments).
  3. Provide a simple version printer for this meta module.
  4. Provide a small debug tag API (set/clear/has).
"""

from __future__ import annotations

import datetime
from load_command_module import load_command_module


# Load the incommon printenv module once at import time
_PRINTENV_MODULE = load_command_module("printenv")
_Z_MODULE = load_command_module("Z")


# Meta module version
_major = 1
_minor = 1
def version_print() -> None:
  """
  Print the meta module version as MAJOR.MINOR.
  """
  print(f"{_major}.{_minor}")


# Debug tag set and helpers
_debug = set([
])


def debug_set(tag: str) -> None:
  """
  Add a debug tag to the meta debug set.
  """
  _debug.add(tag)


def debug_clear(tag: str) -> None:
  """
  Remove a debug tag from the meta debug set, if present.
  """
  _debug.discard(tag)


def debug_has(tag: str) -> bool:
  """
  Return True if the given debug tag is present.
  """
  return tag in _debug


# Touch the default tag once so static checkers do not complain about
# unused helpers when imported purely for side-effects.
debug_has("Command")


def printenv() -> int:
  """
  Call the imported printenv() work function with default arguments:
    - no null termination
    - no newline quoting
    - no specific names (print full environment)
    - prog name 'printenv'
  """
  return _PRINTENV_MODULE.printenv(
    False      # null_terminate
    ,False     # quote_newlines
    ,[]        # names
    ,"printenv"
  )


def z_format_mtime(
  mtime: float
) -> str:
  """
  Format a POSIX mtime (seconds since epoch, UTC) using the Z module.

  Uses Z.ISO8601_FORMAT and Z.make_timestamp(dt=...).
  """
  dt = datetime.datetime.fromtimestamp(mtime, datetime.timezone.utc)
  return _Z_MODULE.make_timestamp(
    fmt=_Z_MODULE.ISO8601_FORMAT
    ,dt=dt
  )
