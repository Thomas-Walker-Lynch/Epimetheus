#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-

"""
rt_property_manager

Process-local property attachment with:
  - weak identity for Python runtime instances
  - explicit identities for syntax instances and properties

Notes:
  - ProcessLocalId values are not meant to be serialized or persisted.
"""

from .PropertyManager import PropertyManager
