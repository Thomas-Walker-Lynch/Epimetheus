#!/usr/bin/env python3
import sys
from Symbol import Symbol

try:
  import TM_module
except ImportError:
  print("Error: Import failed. Run 'python3 setup.py build_ext --inplace'")
  sys.exit(1)

# ==========================================
# 1. The TM Stub (Callable Proxy)
# ==========================================

class TM_Factory_Stub:
  """
  A Proxy object that acts as both:
  1. The Namespace Root (holding .feature)
  2. The Callable Factory (delegating to an implementation)
  """
  def __init__(self):
    self._impl = None
    
  def __call__(self, *args, **kwargs):
    if self._impl is None:
      raise TypeError("TM factory logic not yet bound.")
    return self._impl(*args, **kwargs)

# Create the persistent TM object immediately
TM = TM_Factory_Stub()

# ==========================================
# 2. Define Features (Directly on TM)
# ==========================================

# Create the namespace root
TM.feature = Symbol()

# Allocating features directly onto the persistent object.
# Now we don't need temporary variables like _SR or _L.
TM.feature.SR = TM.feature.alloc() # Step Right
TM.feature.ND = TM.feature.alloc() # Non-Destructive
TM.feature.L  = TM.feature.alloc() # Mirror View

# ==========================================
# 3. The Implementation Logic
# ==========================================

def _tm_implementation(data_obj ,feature_seq=None):
  """
  The actual factory logic.
  Now allows us to refer to 'TM.feature' directly inside the function.
  """
  # Optimization: Default Case
  if not feature_seq: return TM_module.TM_Array_SR_ND(data_obj)

  # Normalize
  fs = set(feature_seq)

  # Remove Defaults (Referring to the authoritative symbols directly)
  fs.discard(TM.feature.SR)
  fs.discard(TM.feature.ND)

  # Check Empty
  if not fs: return TM_module.TM_Array_SR_ND(data_obj)

  # Check Mirror View
  if TM.feature.L in fs:
    fs.remove(TM.feature.L)
    if not fs: return TM_module.TM_Array_ND(data_obj)

  # Unknowns
  raise ValueError(f"Unrecognized features: {fs}")

# ==========================================
# 4. Bind the Logic
# ==========================================

TM._impl = _tm_implementation
