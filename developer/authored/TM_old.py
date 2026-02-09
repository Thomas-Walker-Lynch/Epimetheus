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
  PARKED = auto()

class Topology(Enum):
  CIRCLE = auto()
  LINEAR_RIGHT = auto()
  LINEAR_OPEN = auto()
  NULL = auto()
  SEGMENT = auto()

# The Factory
TM = TM_module.FastTM

# ==========================================
# 2. TM Workspace Functions
# ==========================================
class TM_workspace:
  @staticmethod
  def head_on_same_cell(tm1, tm2):
    """
    Predicate: Returns true if entangled machines are on the same cell.
    Requires: tm1 and tm2 share the same tape.
    """
    if tm1.empty() or tm2.empty(): return False
    # TODO: Verify entanglement identity via C-extension property if needed.
    return tm1.address() == tm2.address()

  @staticmethod
  def step_tandem(tm1, tm2):
    if tm1.empty() or tm2.empty(): return
    tm1.s()
    tm2.s()

# ==========================================
# 3. Status Wrapper (TMS)
# ==========================================

class TMS:
  def __init__(self, data_obj, features=None):
    """
    TMS Constructor.
    Args:
      data_obj: MANDATORY. List or existing TM.
    """
    # 1. Wrap Existing (Entangled) TM
    if hasattr(data_obj, '__class__') and data_obj.__class__.__module__ == 'TM_module':
         self.tm = data_obj
         self._stat = Status.ACTIVE
    
    # 2. Handle Empty List (Demotion to Empty Status)
    #    The C-TM rejects empty lists, so we handle it here.
    elif isinstance(data_obj, list) and len(data_obj) == 0:
         self.tm = None
         self._stat = Status.EMPTY

    # 3. Create New Active TM via Factory
    else:
         self.tm = TM(data_obj, features)
         self._stat = Status.ACTIVE

  # --- Base Methods ---
  def r(self):
    if self.empty(): raise RuntimeError("Attempted to read from EMPTY machine.")
    return self.tm.r()
    
  def rn(self, n): return self.tm.rn(n)
  def w(self, v): return self.tm.w(v)
  def wn(self, v): return self.tm.wn(v)
  
  def s(self):
    if self.empty(): raise RuntimeError("Attempted to step EMPTY machine.") 
    return self.tm.s()
    
  def sn(self, n): return self.tm.sn(n)
  
  def address(self): return 0 if self.empty() else self.tm.address()
  def len(self): return 0 if self.empty() else self.tm.len()

  # --- Features ---
  def aR(self, v): 
    if self.empty():
      # Promotion: Empty -> Active
      # We must assume implicit list container for now
      self.tm = TM([v]) 
      self._stat = Status.ACTIVE
    else:
      self.tm.aR(v)
  
  # --- Meta ---
  def e(self):
    return TMS(self.tm.e()) if not self.empty() else TMS([])

  def empty(self): return self._stat == Status.EMPTY
  def rightmost(self): return True if self.empty() else self.tm.rightmost()

# ==========================================
# 4. Region Machine
# ==========================================

class RegionMachine:
  def __init__(self, t_active):
    """
    Constructs a Region Machine from a single Active TM.
    Initializes Left/Right boundaries to the current Active position.
    """
    self.t_active = t_active
    self.t_left = t_active.e()  # Left Boundary
    self.t_right = t_active.e() # Right Boundary
  
  # --- Bounds Management ---
  def qL(self):
    return TM_workspace.head_on_same_cell(self.t_active, self.t_left)

  def qR(self):
    return TM_workspace.head_on_same_cell(self.t_active, self.t_right)

  def sR(self):
    # Cue to Right Boundary (Simulated via address copying or step loop)
    # Since we lack random access 'cue' in SR_ND, we might need 's' loop
    # or direct address manipulation if supported.
    # For now, we rely on the implementation matching the head.
    pass # Todo: Implement cue via delta steps

  # --- Navigation (Guarded) ---
  def s(self):
    if self.qR():
      raise RuntimeError("Region Boundary Violation: Cannot step past Right Bound.")
    self.t_active.s()
    
  def r(self):
    return self.t_active.r()
