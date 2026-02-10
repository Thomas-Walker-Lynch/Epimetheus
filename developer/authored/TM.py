#!/usr/bin/env python3
import sys

try:
  import TM_module
except ImportError:
  print("Error: Import failed. Run 'python3 setup.py build_ext --inplace'")
  sys.exit(1)

# ==========================================
# TM Command Language (Explicit Types)
# ==========================================

# Import all C-defined types into this namespace
_this_module = sys.modules[__name__]
for name in dir(TM_module):
    if name.startswith("TM_") or name.startswith("TMA_"):
        setattr(_this_module, name, getattr(TM_module, name))

# ==========================================
# Defaults (Aliasing)
# ==========================================

# Pattern: TM_[Container]_[Direction]_[Entanglement]
# Default Container: Arr
# Default Entanglement: ND
# Default Direction: SR

# --- Global Default ---
TM = TM_module.TM_Arr_SR_ND

# --- Container Defaults (Dir=SR, Ent=ND) ---
TM_Arr  = TM_module.TM_Arr_SR_ND
TM_ArrV = TM_module.TM_ArrV_SR_ND
TM_Gr   = TM_module.TM_Gr_SR_ND
TM_Glr  = TM_module.TM_Glr_SR_ND
TM_Set  = TM_module.TM_Set_SR_ND
TM_Map  = TM_module.TM_Map_SR_ND
TM_MapK = TM_module.TM_MapK_SR_ND
TM_MapV = TM_module.TM_MapV_SR_ND
TM_ASCII= TM_module.TM_ASCII_SR_ND
TM_UTF8 = TM_module.TM_UTF8_SR_ND
TM_BCD  = TM_module.TM_BCD_SR_ND

# --- Direction Defaults (Cont=Arr, Ent=ND) ---
TM_SR = TM_module.TM_Arr_SR_ND
TM_SL = TM_module.TM_Arr_SL_ND

# --- Entanglement Defaults (Cont=Arr, Dir=SR) ---
TM_ND = TM_module.TM_Arr_SR_ND
TM_SO = TM_module.TM_Arr_SR_SO
TM_EA = TM_module.TM_Arr_SR_EA

# --- Common Partials ---
TM_Arr_SL = TM_module.TM_Arr_SL_ND
TM_ASCII_SL = TM_module.TM_ASCII_SL_ND
