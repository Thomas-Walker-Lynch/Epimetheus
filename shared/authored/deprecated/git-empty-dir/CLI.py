#!/usr/bin/env python3
# ----------------------------------------------------------------------
# git-empty-dir :: list/mark/clean empty directories, .gitignore aware
# ----------------------------------------------------------------------

import sys
import os
from pathlib import Path

# The source_sync GitIgnore parser is inside the unpacked tool.
# We assume this directory structure:
#   git-empty-dir/
#     CLI.py
#     source_sync/
#       GitIgnore.py
#
# That mirrors how your harmony sync tool is structured.

# Adjust import path so we can load source_sync.*
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from source_sync.GitIgnore import GitIgnore  # type: ignore


# ----------------------------------------------------------------------
# helpers
# ----------------------------------------------------------------------

def load_gitignore_tree(root: Path):
  """
  Build a GitIgnore instance rooted at <root>.
  """
  return GitIgnore(str(root))

def is_empty_dir(path: Path) -> bool:
  """
  A directory is empty if it contains no files or subdirectories.
  (Hidden files count; .gitignored children are irrelevant because
  behavior here should reflect real filesystem emptiness.)
  """
  try:
    for _ in path.iterdir():
      return False
    return True
  except PermissionError:
    # treat as non-empty: safer than aborting
    return False


def has_mark(path: Path, mark_file: str) -> bool:
  return (path / mark_file).exists()


def sorted_dirs(root: Path):
  """
  Produce a list of all directories under root, in parent-before-child order.
  Sort rule:
    1. by path length
    2. then lexicographically
  """
  all_dirs = []
  for p in root.rglob("*"):
    if p.is_dir():
      all_dirs.append(p)

  return sorted(
    all_dirs
    ,key = lambda p: (len(p.parts), str(p))
  )


# ----------------------------------------------------------------------
# traversal
# ----------------------------------------------------------------------

def visible_dirs(root: Path, ignore_tree, mark_file: str):
  """
  Yield all dirs under root, applying:
    - skip .git
    - apply .gitignore rules (if a dir is ignored, do not descend)
    - parent-before-child ordering
  """
  for d in sorted_dirs(root):
    rel = d.relative_to(root)

    if rel == Path("."):
      continue

    # skip .git explicitly
    if d.name == ".git":
      continue

    # .gitignore filtering
    if ignore_tree.check(str(rel)) == "Ignore":
      continue

    yield d


# ----------------------------------------------------------------------
# actions
# ----------------------------------------------------------------------

def action_list(root, ignore_tree, mark_file, mode):
  """
  mode ∈ {"empty","marked","all"}
  """
  for d in visible_dirs(root, ignore_tree, mark_file):
    if mode == "all":
      print(d.relative_to(root))
      continue

    if mode == "marked":
      if has_mark(d, mark_file):
        print(d.relative_to(root))
      continue

    if mode == "empty":
      if is_empty_dir(d):
        print(d.relative_to(root))
      continue


def action_mark(root, ignore_tree, mark_file, mode):
  """
  mode ∈ {"empty","all"}
  """
  for d in visible_dirs(root, ignore_tree, mark_file):
    if mode == "empty" and not is_empty_dir(d):
      continue
    try:
      (d / mark_file).touch(exist_ok=True)
    except Exception:
      pass


def action_clean(root, ignore_tree, mark_file, mode):
  """
  mode ∈ {"nonempty","all"}
  """
  for d in visible_dirs(root, ignore_tree, mark_file):
    m = d / mark_file
    if not m.exists():
      continue

    if mode == "nonempty":
      if is_empty_dir(d):
        continue

    try:
      m.unlink()
    except Exception:
      pass


# ----------------------------------------------------------------------
# usage
# ----------------------------------------------------------------------

USAGE = """
usage:
  git-empty-dir (list|mark|clean) [all|marked|empty] [file-<name>]
  git-empty-dir help
  git-empty-dir usage

defaults:
  mark-file = .gitkeep
  ignores .git
  follows .gitignore (no descent into ignored dirs)

examples:
  git-empty-dir list
  git-empty-dir list marked file-.githolder
  git-empty-dir mark
  git-empty-dir clean all
"""


# ----------------------------------------------------------------------
# CLI
# ----------------------------------------------------------------------

def CLI(argv):
  if len(argv) == 0:
    print(USAGE)
    return 0

  cmd = argv[0]

  if cmd in ("help","usage"):
    print(USAGE)
    return 0

  # command
  if cmd not in ("list","mark","clean"):
    print(f"unknown command: {cmd}")
    print(USAGE)
    return 1

  # submode
  mode = None
  mark_file = ".gitkeep"

  for a in argv[1:]:
    if a.startswith("file-"):
      mark_file = a[5:]
      continue

    if a in ("all","empty","marked"):
      mode = a
      continue

    print(f"unknown argument: {a}")
    print(USAGE)
    return 1

  # defaults
  if cmd == "list":
    if mode is None:
      mode = "empty"
  elif cmd == "mark":
    if mode is None:
      mode = "empty"
  elif cmd == "clean":
    if mode is None:
      mode = "nonempty"

  root = Path(".").resolve()
  ignore_tree = load_gitignore_tree(root)

  if cmd == "list":
    action_list(root, ignore_tree, mark_file, mode)

  elif cmd == "mark":
    if mode == "all":
      action_mark(root, ignore_tree, mark_file, "all")
    else:
      action_mark(root, ignore_tree, mark_file, "empty")

  elif cmd == "clean":
    if mode == "all":
      action_clean(root, ignore_tree, mark_file, "all")
    else:
      action_clean(root, ignore_tree, mark_file, "nonempty")

  return 0


if __name__ == "__main__":
  sys.exit(CLI(sys.argv[1:]))
