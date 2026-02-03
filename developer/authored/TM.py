#!/usr/bin/env python3
import sys
from enum import Enum ,auto

try:
  import TM_module
except ImportError:
  print("Error: Import failed. Run 'python3 setup.py build_ext --inplace'")
  sys.exit(1)

# ==========================================
# 1. Enums
# ==========================================

LM = "LM"
RM = "RM"

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
# 2. The Machine (Direct Alias)
# ==========================================

# The Safety Logic is now 100% in C.
# We just alias the type.
TM = TM_module.FastTM

# ==========================================
# 3. Status Wrapper
# ==========================================

class TM2:
  def __init__(self ,data_obj=None):
    if data_obj:
      self.tm = TM(data_obj)
      self._stat = Status.ACTIVE
    else:
      self.tm = None
      self._stat = Status.EMPTY

  def r(self): return self.tm.r()
  def rn(self ,n_val): return self.tm.rn(n_val)
  def w(self ,val_obj): return self.tm.w(val_obj)
  def wn(self ,val_obj): return self.tm.wn(val_obj)
  def s(self): return self.tm.s()
  def sn(self ,n_val): return self.tm.sn(n_val)
  def ls(self): return self.tm.ls()
  def lsn(self ,n_val): return self.tm.lsn(n_val)
  def d(self): return self.tm.d()
  def dn(self ,n_val): return self.tm.dn(n_val)
  def esd(self): return self.tm.esd()
  def esdn(self ,n_val): return self.tm.esdn(n_val)
  def aL(self ,val_obj): return self.tm.aL(val_obj)
  def aR(self ,val_obj): return self.tm.aR(val_obj)
  def e(self): return TM2(self.tm.e()) if not self.empty() else TM2(None)
    
  def empty(self): return self._stat == Status.EMPTY
  def rightmost(self): return True if self.empty() else self.tm.rightmost()
  def leftmost(self): return True if self.empty() else self.tm.leftmost()
  def address(self): return 0 if self.empty() else self.tm.address()
  def topology(self): return Topology.NULL if self.empty() else Topology.SEGMENT

# ==========================================
# 4. Verification
# ==========================================

def CLI():
  print("--- TM Full C-Entanglement Verification ---")
  
  # 1. Setup
  t1 = TM2(['A' ,'B' ,'C'])
  t2 = t1.e() # Entangle (C handles registration)
  
  print(f"T1 Addr: {t1.address()}")
  print(f"T2 Addr: {t2.address()}")
  
  # 2. Move T2 to danger zone
  t2.s() # T2 on 'B'
  print(f"T2 Step -> {t2.r()}")

  # 3. Trigger Entanglement Check (in C)
  print("T1 attempting to delete 'B' (esd)...")
  try:
    t1.esd() # Should fail
  except RuntimeError as e:
    print(f"Caught C-Level Exception: {e}")

if __name__ == "__main__":
  CLI()

