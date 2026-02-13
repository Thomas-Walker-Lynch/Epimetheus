# TMS.py
# RT format: 2-space indent, PascalCase type names, snake_case identifiers.

from enum import Enum, auto

class Status(Enum):
  EMPTY_TYPED = auto()    # No instance, knows type.
  EMPTY_STASHED = auto()  # Instance exists (1 zombie cell), logically empty.
  PARKED = auto()         # Instance exists, head virtual -1.
  SINGLETON = auto()      # Instance exists, size 1, Head at 0.
  ACTIVE = auto()         # Instance exists, size > 1 OR Head > 0.


class TMS:
  """
  Turing Machine with Status (Second Order Machine).
  - Projective Parked State (Connects to both ends).
  - Late Binding & Zombie Cell optimization.
  - Partial Direct Binding for performance.
  """
  __slots__ = (
    "tm"
    ,"tm_type"
    ,"_status"
    # VTable Methods
    ,"s"
    ,"Ls"
    ,"r"
    ,"w"
    ,"d"
    ,"append"
    ,"cue_leftmost"
    ,"cue_rightmost"
    ,"park"
    ,"dealloc"
    ,"is_empty"
  )

  def __init__(self ,tm=None ,tm_type=None):
    self.tm = None
    self.tm_type = None

    if(tm is not None):
      self.tm = tm
      self.tm_type = type(tm)
      self._change_state(Status.PARKED)
    elif(tm_type is not None):
      self.tm_type = tm_type
      self._change_state(Status.EMPTY_TYPED)
    else:
      self._change_state(Status.EMPTY_TYPED)

  @property
  def status(self):
    return self._status

  def _change_state(self ,new_status):
    self._status = new_status
    
    # ---------------------------------------------------------
    # STATE: ACTIVE
    # ---------------------------------------------------------
    if(new_status == Status.ACTIVE):
      # Direct Binding (Max Performance)
      self.r = self.tm.r
      self.w = self.tm.w
      self.append = self._active_append
      
      # Navigation: Wrapped Binding (Guarded)
      self.s = self._active_s
      self.Ls = self._active_Ls
      
      self.d = self._active_d
      self.cue_leftmost = self._active_cue_leftmost
      self.cue_rightmost = self._active_cue_rightmost
      self.park = self._active_park
      self.dealloc = self._generic_dealloc
      self.is_empty = self._return_false
      
    # ---------------------------------------------------------
    # STATE: SINGLETON (Size == 1, Head == 0)
    # ---------------------------------------------------------
    elif(new_status == Status.SINGLETON):
      self.r = self.tm.r
      self.w = self.tm.w
      
      # Navigation
      self.s = self._singleton_s      # Falls off 0 -> Active
      self.Ls = self._singleton_Ls    # Falls off 0 -> Parked
      
      self.d = self._singleton_d      # Becomes Stashed
      self.append = self._singleton_append
      
      self.cue_leftmost = self._noop  # Already at 0
      self.cue_rightmost = self._noop # Already at 0 (Rightmost=Leftmost)
      self.park = self._active_park
      self.dealloc = self._generic_dealloc
      self.is_empty = self._return_false

    # ---------------------------------------------------------
    # STATE: PARKED (Projective, Head Virtual)
    # ---------------------------------------------------------
    elif(new_status == Status.PARKED):
      self.s = self._parked_s     # Enter Left
      self.Ls = self._parked_Ls   # Enter Right (Projective)
      
      self.r = self._error_io
      self.w = self._error_io
      self.d = self._parked_d
      self.append = self._error_append
      
      self.cue_leftmost = self._parked_cue_leftmost
      self.cue_rightmost = self._parked_cue_rightmost
      self.park = self._noop
      self.dealloc = self._generic_dealloc
      self.is_empty = self._return_false

    # ---------------------------------------------------------
    # STATE: EMPTY STASHED
    # ---------------------------------------------------------
    elif(new_status == Status.EMPTY_STASHED):
      self.s = self._error_step
      self.Ls = self._error_step
      self.r = self._error_io
      self.w = self._error_io
      self.d = self._error_op
      
      self.append = self._stashed_append
      
      self.cue_leftmost = self._noop
      self.cue_rightmost = self._noop
      self.park = self._noop
      self.dealloc = self._generic_dealloc
      self.is_empty = self._return_true

    # ---------------------------------------------------------
    # STATE: EMPTY TYPED
    # ---------------------------------------------------------
    elif(new_status == Status.EMPTY_TYPED):
      self.s = self._error_step
      self.Ls = self._error_step
      self.r = self._error_io
      self.w = self._error_io
      self.d = self._error_op
      
      self.append = self._typed_append
      
      self.cue_leftmost = self._noop
      self.cue_rightmost = self._noop
      self.park = self._noop
      self.dealloc = self._noop
      self.is_empty = self._return_true

  # --- ACTIVE wrappers ---

  def _active_s(self):
    # Guard against stepping off the right end
    if(self.tm.qR()):
      raise RuntimeError("Cannot step right: At Rightmost cell.")
    self.tm.s()

  def _active_Ls(self):
    # Guard: If at Leftmost, transition to Parked
    if(self.tm.Lq()):
      self._change_state(Status.PARKED)
    else:
      self.tm.Ls()

  def _active_d(self):
    self.tm.d()
    if(self.tm.is_singleton()):
      self._change_state(Status.SINGLETON)

  def _active_append(self ,val):
    self.tm.append(val)

  def _active_cue_leftmost(self):
    self.tm.cue_leftmost()

  def _active_cue_rightmost(self):
    self.tm.cue_rightmost()

  def _active_park(self):
    self.tm.cue_leftmost()
    self._change_state(Status.PARKED)

  # --- SINGLETON wrappers ---

  def _singleton_s(self):
    # Step Right from Singleton(0).
    # If successful, we are no longer at 0 (or size > 1).
    # Thus, we must transition to ACTIVE.
    self.tm.s()
    self._change_state(Status.ACTIVE)

  def _singleton_Ls(self):
    # Left of Singleton(0) is Parked
    self._change_state(Status.PARKED)

  def _singleton_d(self):
    # Stash zombie
    self._change_state(Status.EMPTY_STASHED)

  def _singleton_append(self ,val):
    self.tm.append(val)
    self._change_state(Status.ACTIVE)

  # --- PARKED wrappers ---

  def _parked_s(self):
    # Enter tape at Leftmost (0)
    self.tm.cue_leftmost()
    if(self.tm.is_singleton()):
      self._change_state(Status.SINGLETON)
    else:
      self._change_state(Status.ACTIVE)

  def _parked_Ls(self):
    # Enter tape at Rightmost (Projective)
    self.tm.cue_rightmost()
    if(self.tm.is_singleton()):
      self._change_state(Status.SINGLETON)
    else:
      self._change_state(Status.ACTIVE)

  def _parked_d(self):
    # Delete Leftmost (right neighbor)
    self.tm.cue_leftmost()
    if(self.tm.is_singleton()):
      self._change_state(Status.EMPTY_STASHED)
    else:
      self.tm.d()
      if(self.tm.is_singleton()):
        self._change_state(Status.SINGLETON)
      else:
        self._change_state(Status.ACTIVE)

  def _parked_cue_leftmost(self):
    self._parked_s()

  def _parked_cue_rightmost(self):
    self._parked_Ls()

  # --- SPECIAL APPENDS ---

  def _stashed_append(self ,val):
    self.tm.w(val)
    self._change_state(Status.SINGLETON)

  def _typed_append(self ,val):
    if(self.tm_type is None):
      raise RuntimeError("No TM type known.")
    self.tm = self.tm_type(val)
    self._change_state(Status.SINGLETON)

  # --- GENERIC ---

  def _generic_dealloc(self):
    if(self.tm is not None):
      self.tm_type = type(self.tm)
      self.tm = None
    self._change_state(Status.EMPTY_TYPED)

  def _return_true(self): return True
  def _return_false(self): return False
  def _noop(self): pass
  
  def _error_io(self ,*args):
    raise RuntimeError(f"I/O illegal in {self._status.name} state.")
    
  def _error_step(self):
    raise RuntimeError(f"Stepping illegal in {self._status.name} state.")

  def _error_op(self):
    raise RuntimeError(f"Operation illegal in {self._status.name} state.")
  
  def _error_append(self ,val):
     raise RuntimeError(f"Cannot append in {self._status.name} state.")
