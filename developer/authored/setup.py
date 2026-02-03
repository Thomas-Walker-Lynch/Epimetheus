from setuptools import setup ,Extension

module_obj = Extension(
  "TM_module"
  ,sources=["TM_module.c"]
)

setup(
  name="TM_module"
  ,version="1.0"
  ,description="Fast Tape Machine C Extension"
  ,ext_modules=[module_obj]
)
