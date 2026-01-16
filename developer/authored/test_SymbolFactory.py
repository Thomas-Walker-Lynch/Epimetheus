#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-

"""
Tests for Symbol.Instance constraints.
"""

import unittest
from Symbol import Symbol

class TestSymbol(unittest.TestCase):

  def test_null_symbol_exists(self):
    """
    Requirement: Symbol.null exists and is Token 0.
    """
    null_sym = Symbol.null
    self.assertIsNotNone(null_sym)
    self.assertEqual(null_sym.name ,"Null")
    self.assertEqual(null_sym.doc ,"The Null Symbol")

  def test_distinctness(self):
    """
    Requirement: Two distinct originals will always be not equal.
    """
    s1 = Symbol.make()
    s2 = Symbol.make()

    self.assertNotEqual(s1 ,s2)
    self.assertIsNot(s1 ,s2)
    self.assertNotEqual(s1 ,Symbol.null)

  def test_name_immutability(self):
    """
    Requirement: Name can be set once, but never changed.
    """
    # Case 1: Set at creation
    s1 = Symbol.make(name="Alpha")
    self.assertEqual(s1.name ,"Alpha")

    # Attempt to change
    with self.assertRaises(RuntimeError):
      s1.name = "Beta"

    # Case 2: Late binding
    s2 = Symbol.make()
    self.assertIsNone(s2.name)

    s2.name = "Gamma"
    self.assertEqual(s2.name ,"Gamma")

    # Attempt to change after late bind
    with self.assertRaises(RuntimeError):
      s2.name = "Delta"

  def test_doc_immutability(self):
    """
    Requirement: Doc can be set once, but never changed.
    """
    # Case 1: Set at creation
    s1 = Symbol.make(doc="Original Doc")
    self.assertEqual(s1.doc ,"Original Doc")

    with self.assertRaises(RuntimeError):
      s1.doc = "New Doc"

    # Case 2: Late binding
    s2 = Symbol.make()
    self.assertEqual(s2.doc ,"") # Default is empty string

    s2.doc = "Late Doc"
    self.assertEqual(s2.doc ,"Late Doc")

    with self.assertRaises(RuntimeError):
      s2.doc = "Changed Doc"

  def test_doc_empty_string_behavior(self):
    """
    Requirement: Setting doc to "" is ignored and does not count as 'setting' it.
    """
    s1 = Symbol.make()

    # Setting empty string should be a no-op
    s1.doc = ""
    self.assertEqual(s1.doc ,"")

    # Should still be able to set it later because "" didn't lock it
    s1.doc = "Real Doc"
    self.assertEqual(s1.doc ,"Real Doc")

    # NOW it is locked
    with self.assertRaises(RuntimeError):
      s1.doc = "Trying to change"

  def test_property_access(self):
    """
    Requirement: Read/Write via properties.
    """
    s = Symbol.make()
    s.name = "Velocity"
    s.doc = "m/s"

    self.assertEqual(s.name ,"Velocity")
    self.assertEqual(s.doc ,"m/s")
    self.assertIsInstance(s ,Symbol.Instance)

  def test_opaque_representation(self):
    """
    Requirement: repr() reveals no internal token.
    """
    s1 = Symbol.make()
    self.assertEqual(repr(s1) ,"<Symbol.Instance>")


if __name__ == '__main__':
  unittest.main()
