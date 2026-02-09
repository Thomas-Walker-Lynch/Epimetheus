#!/usr/bin/env python3
import sys

try:
  import TM_module
except ImportError:
  print("Error: Import failed. Run 'python3 setup.py build_ext --inplace'")
  sys.exit(1)

class Features:
  # No features currently supported for Array TM
  pass

# The First Order TM Factory
TM = TM_module.TM
