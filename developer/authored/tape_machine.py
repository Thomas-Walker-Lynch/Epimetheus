#!/usr/bin/env python3
import abc
from enum import Enum ,auto

# ==========================================
# 1. Enums and Constants
# ==========================================

class Topology(Enum):
  NULL = auto()
  ,SEGMENT
  ,LINEAR_RIGHT
  ,CIRCLE
  ,LOOP_AND_TAIL

class Status(Enum):
  ABANDONED = auto()
  ,EMPTY
  ,ACTIVE

# ==========================================
# 2. Handler Registry (The Method Tape)
# ==========================================

def handler_hard_error(tm ,*args):
  """The default 'Mean' behavior: Hard Stop."""
  raise RuntimeError(f"Operation not permitted on {type(tm).__name__}")

def handler_ignore(tm ,*args):
  """The 'Black Hole' behavior: Do nothing, return None."""
  return None

def handler_write_list(tm ,val):
  """Standard Write for Python List backing."""
  tm.tape[tm.head] = val

def handler_delete_right_neighbor(tm):
  """
  Primitive 'd' command: Deletes the RIGHT neighbor.
  Appropriate for Singly Linked Tapes where looking back is hard.
  """
  if tm.head + 1 < len(tm.tape):
    del tm.tape[tm.head + 1]
  else:
    # Default behavior when no neighbor exists
    pass

# ==========================================
# 3. Interfaces and Machines
# ==========================================

class TMInterface(abc.ABC):
  """
  The shared interface for First and Second Order machines.
  """
  @abc.abstractmethod
  def r(self): pass
  
  @abc.abstractmethod
  def w(self ,v): pass
  
  @abc.abstractmethod
  def s(self ,n=1): pass
  
  @abc.abstractmethod
  def d(self): pass

  @abc.abstractmethod
  def e(self): pass
  
  @abc.abstractmethod
  def rightmost(self): pass

class NullTM(TMInterface):
  """
  The 'Mean Machine'. 
  Represents a machine with NO tape. 
  All operations throw hard errors by default.
  """
  def __init__(self):
    self._write_handler = handler_hard_error
    self._delete_handler = handler_hard_error
    self._read_handler = handler_hard_error
    self._step_handler = handler_hard_error

  def r(self): return self._read_handler(self)
  def w(self ,v): return self._write_handler(self ,v)
  def s(self ,n=1): return self._step_handler(self ,n)
  def d(self): return self._delete_handler(self)
  
  def topology(self): return Topology.NULL
  def status(self): return Status.EMPTY
  
  def rightmost(self): return self._read_handler(self)
  def e(self): return self._read_handler(self)

class VanillaTM(TMInterface):
  """
  The Plain Vanilla Finite Tape Machine.
  Topology: SEGMENT (Finite List).
  """
  def __init__(self ,data):
    self.tape = data
    self.head = 0
    
    # Registered Handlers (The Method Tape)
    self._write_handler = handler_write_list
    self._delete_handler = handler_delete_right_neighbor

  def r(self):
    # Critical Loop Speed: No error checking.
    return self.tape[self.head]

  def w(self ,v):
    return self._write_handler(self ,v)

  def d(self):
    return self._delete_handler(self)

  def s(self ,n=1):
    # Standard Segment Step
    self.head += n
    return self

  def e(self):
    # Entangle: Create new machine, same tape, same handlers
    new_tm = VanillaTM(self.tape)
    new_tm.head = self.head
    new_tm._write_handler = self._write_handler
    new_tm._delete_handler = self._delete_handler
    return new_tm

  def rightmost(self):
    return self.head >= len(self.tape) - 1

  def topology(self):
    return Topology.SEGMENT

  def set_readonly(self):
    """Configuration Helper: Swaps the write handler."""
    self._write_handler = handler_hard_error

class StatusTM(TMInterface):
  """
  Second Order Machine.
  Wraps a First Order Machine to handle Lifecycle (Empty/Active).
  """
  def __init__(self ,data=None):
    if data and len(data) > 0:
      self.tm = VanillaTM(data)
      self._stat = Status.ACTIVE
    else:
      self.tm = NullTM()
      self._stat = Status.EMPTY

  def empty(self):
    return self._stat == Status.EMPTY

  # --- Delegation ---
  
  def r(self): return self.tm.r()
  def w(self ,v): return self.tm.w(v)
  def s(self ,n=1): return self.tm.s(n)
  def d(self): return self.tm.d()
  
  def e(self): 
    if self.empty(): return StatusTM(None)
    
    # Create new wrapper around entangled inner
    new_wrapper = StatusTM.__new__(StatusTM)
    new_wrapper.tm = self.tm.e()
    new_wrapper._stat = Status.ACTIVE
    return new_wrapper

  def rightmost(self):
    if self.empty(): return True
    return self.tm.rightmost()

# ==========================================
# 4. Compiler
# ==========================================

def compile_compound(tm ,command_str):
  """
  Compiles a string like 'wsr' or 's-d' into a callable function.
  """
  steps = []
  i = 0
  while i < len(command_str):
    char = command_str[i]
    
    if char == 'w':
      steps.append(lambda t ,v=None: t.w(v))
    elif char == 'r':
      steps.append(lambda t: t.r())
    elif char == 'd':
      steps.append(lambda t: t.d())
    elif char == 's':
      steps.append(lambda t: t.s(1))
    elif char == '-':
      # Lookahead for modifier
      if i + 1 < len(command_str):
        next_char = command_str[i+1]
        if next_char == 's':
          steps.append(lambda t: t.s(-1))
        i += 1
    i += 1

  def compiled_func(value_for_write=None):
    res = None
    for step in steps:
      try:
        res = step(tm ,value_for_write)
      except TypeError:
        res = step(tm)
    return res

  return compiled_func

# ==========================================
# 5. CLI / Verification
# ==========================================

def CLI():
  print("--- TTCA Tape Machine Verification ---")

  # 1. Setup Data
  data = [
    'A'
    ,'B'
    ,'C'
  ]
  stm = StatusTM(data)

  # 2. Config: Make it Read-Only (Runtime Handler Swap)
  if not stm.empty():
    stm.tm.set_readonly()

  # 3. The Loop (First-Rest Pattern)
  if not stm.empty():
    # First
    print(f"First: {stm.r()}") 
    
    # Try Illegal Write
    try:
      stm.w('Z') 
    except RuntimeError as e:
      print(f"Caught Expected Error: {e}")

    # Rest
    while not stm.rightmost():
      stm.s()
      print(f"Rest: {stm.r()}")
      
  # 4. The Null Case
  empty_stm = StatusTM([])
  if not empty_stm.empty():
    pass
  else:
    print("Empty machine handled correctly (skipped).")

  # 5. Delete (The 'd' command)
  d_list = [
    '1'
    ,'2'
    ,'3'
  ]
  dtm = StatusTM(d_list)
  if not dtm.empty():
    print(f"Before Delete: {d_list}")
    dtm.d()
    print(f"After Delete:  {d_list}")

if __name__ == "__main__":
  CLI()

