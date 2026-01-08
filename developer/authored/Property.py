#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-


"""
Property

A Property is itself an entity (it has an Identity id) so that:
  - properties can have properties
  - properties can be members of semantic sets
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Optional ,Tuple

from .ProcessLocalId import ProcessLocalId


@dataclass(frozen=True ,slots=True)
class Property:
  id: ProcessLocalId
  name_path: Tuple[str ,...]
  doc: str = ""

  def __repr__(self) -> str:
    # name_path is safe to reveal; id token is not.
    return f"<Property {'.'.join(self.name_path)!r}>"
