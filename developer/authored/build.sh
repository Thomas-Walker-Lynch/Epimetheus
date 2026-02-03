#!/usr/bin/env bash
set -euo pipefail

# Compile with FACE and IMPL defined to generate the full library
python3 setup.py build_ext --inplace

