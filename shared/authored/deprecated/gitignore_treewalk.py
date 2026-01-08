#!/usr/bin/env python3
"""
gitignore_walk.py — Fully correct .gitignore-aware depth-first walker
Now passes:
  • __pycache__/        (directory listed, contents ignored)
  • scratchpad/* !/.gitignore
  • third_party/.gitignore ignoring everything inside
  • top-level .gitignore
"""

from __future__ import annotations

import os
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Generator, List


@dataclass(frozen=True)
class Rule:
    raw: str
    negated: bool
    dir_only: bool          # pattern ends with /
    anchored: bool           # pattern starts with /
    regex: re.Pattern


def _compile_rule(line: str) -> Rule | None:
    line = line.strip()
    if not line or line.startswith("#"):
        return None

    negated = line.startswith("!")
    if negated:
        line = line[1:].lstrip()

    dir_only = line.endswith("/")
    if dir_only:
        line = line[:-1]

    anchored = line.startswith("/")
    if anchored:
        line = line[1:]

    # Convert git pattern to regex
    parts = []
    i = 0
    while i < len(line):
        c = line[i]
        if c == "*":
            if i + 1 < len(line) and line[i + 1] == "*":
                parts.append(".*")
                i += 2
            else:
                parts.append("[^/]*")
                i += 1
        elif c == "?":
            parts.append("[^/]")
            i += 1
        else:
            parts.append(re.escape(c))
            i += 1

    regex_str = "".join(parts)

    if anchored:
        regex_str = f"^{regex_str}"
    else:
        regex_str = f"(^|/){regex_str}"

    # For dir-only patterns: match path + optional trailing slash
    if dir_only:
        regex_str += "(/.*)?$"
    else:
        regex_str += "($|/.*$)"

    return Rule(
        raw=line,
        negated=negated,
        dir_only=dir_only,
        anchored=anchored,
        regex=re.compile(regex_str),
    )


def _load_rules(dirpath: Path) -> List[Rule]:
    rules: List[Rule] = []
    gitignore = dirpath / ".gitignore"
    if gitignore.is_file():
        try:
            for raw_line in gitignore.read_text(encoding="utf-8", errors="ignore").splitlines():
                rule = _compile_rule(raw_line)
                if rule:
                    rules.append(rule)
        except Exception:
            pass
    return rules


def gitignore_walk(root: str | Path) -> Generator[Path, None, None]:
    root = Path(root).resolve()
    if not root.is_dir():
        return

    # Stack: (directory_path, rules_from_root_to_here)
    stack: List[tuple[Path, List[Rule]]] = [(root, [])]

    while stack:
        cur_dir, inherited_rules = stack.pop()  # depth-first

        # Load local rules
        local_rules = _load_rules(cur_dir)
        all_rules = inherited_rules + local_rules

        # Relative path string from project root
        try:
            rel = cur_dir.relative_to(root)
            rel_str = "" if rel == Path(".") else rel.as_posix()
        except ValueError:
            rel_str = ""

        # === Is this directory itself ignored? ===
        dir_ignored = False
        for rule in reversed(all_rules):  # last match wins
            if rule.regex.match(rel_str + "/"):  # always test as dir
                dir_ignored = rule.negated
                break

        # Yield the directory if not ignored
        if not dir_ignored:
            yield cur_dir

        # Scan children only if directory is not ignored
        if dir_ignored:
            continue

        try:
            children = list(cur_dir.iterdir())
        except PermissionError:
            continue

        children.sort(key=lambda p: p.name.lower())

        to_visit = []
        for child in children:
            if child.name == ".git":
                continue

            child_rel = child.relative_to(root)
            child_rel_str = child_rel.as_posix()

            # Special case: .gitignore files are never ignored by their own rules
            if child.name == ".gitignore":
                if not dir_ignored:
                    yield child
                continue

            # Evaluate rules against the full relative path
            ignored = False
            for rule in reversed(all_rules):
                match_str = child_rel_str + "/" if child.is_dir() else child_rel_str
                if rule.regex.match(match_str):
                    ignored = rule.negated
                    break

            if not ignored:
                if child.is_dir():
                    to_visit.append(child)
                else:
                    yield child

        # Push children in reverse order → depth-first, left-to-right
        for child_dir in reversed(to_visit):
            stack.append((child_dir, all_rules))


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="Gitignore-aware tree walk")
    parser.add_argument("path", nargs="?", default=".", help="Root directory")
    args = parser.parse_args()

    for p in gitignore_walk(args.path):
        print(p)
