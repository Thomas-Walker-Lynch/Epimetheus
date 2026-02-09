#!/usr/bin/env python3
import sys
from enum import Enum, auto

try:
  import TM_module
except ImportError:
  print("Error: Import failed. Run 'python3 setup.py build_ext --inplace'")
  sys.exit(1)

# ==========================================
# 1. Enums & Features
# ==========================================

class Features:
  APPEND_RIGHT = "aR"

class Status(Enum):
  ABANDONED = auto()
  ACTIVE = auto()
  EMPTY = auto()

class Topology(Enum):
  CIRCLE = auto()
  LINEAR_RIGHT = auto()
  LINEAR_OPEN = auto()
  NULL = auto()
  SEGMENT = auto()

# ==========================================
# 2. The Machine (Factory)
# ==========================================

# TM is now the Factory Function from the C module
TM = TM_module.FastTM

# ==========================================
# 3. Status Wrapper (TMS)
# ==========================================

class TMS:
  def __init__(self, data_obj=None, features=None):
    """
    TMS Constructor.
    Args:
      data_obj: Initial data container (list).
      features: List of feature symbols (e.g. [Features.APPEND_RIGHT]).
    """
    if data_obj:
      # Factory Call: C determines the underlying type based on features
      self.tm = TM(data_obj, features)
      self._stat = Status.ACTIVE
    else:
      self.tm = None
      self._stat = Status.EMPTY

  # --- Base Methods (SR_ND) ---
  def r(self): return self.tm.r()
  def rn(self, n): return self.tm.rn(n)
  def w(self, v): return self.tm.w(v)
  def wn(self, v): return self.tm.wn(v)
  def s(self): return self.tm.s()
  def sn(self, n): return self.tm.sn(n)
  
  def address(self): return 0 if self.empty() else self.tm.address()
  def len(self): return 0 if self.empty() else self.tm.len()

  # --- Feature Methods (Delegated) ---
  # If the C-Type doesn't have these, Python raises AttributeError.
  def aR(self, v): return self.tm.aR(v)
  
  # --- Meta ---
  def e(self):
    # Cloning preserves the underlying C-Type (and thus features)
    return TMS(self.tm.e()) if not self.empty() else TMS(None)

  def empty(self): return self._stat == Status.EMPTY
  def rightmost(self): return True if self.empty() else self.tm.rightmost()
  def leftmost(self): return True if self.empty() else (self.tm.head == 0)
  def topology(self): return Topology.NULL if self.empty() else Topology.SEGMENT
