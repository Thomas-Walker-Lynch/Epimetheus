#!/usr/bin/env python3
import abc
from enum import Enum ,auto

# ==========================================
# 1. Enums and Constants
# ==========================================

class Topology(Enum):
  NULL = auto()
  SEGMENT = auto()
  LINEAR_RIGHT = auto()
  CIRCLE = auto()
  LOOP_AND_TAIL = auto()

class Status(Enum):
  ABANDONED = auto()
  EMPTY = auto()
  ACTIVE = auto()

# ==========================================
# 2. Handler Registry (The Method Tape)
# ==========================================

def handler_hard_error(tm ,*args):
  """The default 'Mean' behavior: Hard Stop."""
  raise RuntimeError(f"Operation not permitted on {type(tm).__name__}")

def handler_ignore(tm ,*args):
  """The 'Black Hole' behavior: Do nothing."""
  return None

def handler_write_list(tm ,val):
  """Standard Write for List backing."""
  # Entanglement Accounting Check
  if tm.tape_ref.count > 1:
    raise RuntimeError("Cannot Write: Tape is Entangled (Shared)")
  tm.tape_ref.data[tm.head] = val

def handler_delete_right_neighbor_list(tm):
  """Delete right neighbor in a list."""
  if tm.tape_ref.count > 1:
    raise RuntimeError("Cannot Delete: Tape is Entangled (Shared)")
  
  # Check neighbor existence
  if tm.head + 1 < len(tm.tape_ref.data):
    del tm.tape_ref.data[tm.head + 1]

def handler_write_map(tm ,val):
  """Standard Write for Map backing (updates value for current key)."""
  if tm.tape_ref.count > 1:
    raise RuntimeError("Cannot Write: Tape is Entangled")
  
  key = tm.address()
  tm.tape_ref.data[key] = val

# ==========================================
# 3. Tape Reference (Entanglement Accounting)
# ==========================================

class Tape:
  """
  Holds the actual data and the entanglement reference count.
  """
  def __init__(self ,data):
    self.data = data
    self.count = 1  # Start with 1 owner
    
    # For Maps, we might need a stable key view for stepping
    self.keys_view = None
    if isinstance(data ,dict):
      self.keys_view = list(data.keys())

  def checkout(self):
    self.count += 1
    
  def checkin(self):
    if( self.count > 0 ):
      self.count -= 1

# ==========================================
# 4. The Machines
# ==========================================

class TM_Interface(abc.ABC):
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
  @abc.abstractmethod
  def address(self): pass
  @abc.abstractmethod
  def dismount(self): pass

class TM(TM_Interface):
  """
  The General Entanglement Accounting Machine.
  Specialized for LIST (Sequence) tapes.
  """
  def __init__(self ,data):
    # If data is already a Tape object (from entanglement), use it.
    # Otherwise create new.
    if( isinstance(data ,Tape) ):
      self.tape_ref = data
    else:
      self.tape_ref = Tape(data)
      
    self.head = 0
    
    # Registered Handlers
    self._write_handler = handler_write_list
    self._delete_handler = handler_delete_right_neighbor_list

  def r(self):
    return self.tape_ref.data[self.head]

  def w(self ,v):
    return self._write_handler(self ,v)

  def d(self):
    return self._delete_handler(self)

  def s(self ,n=1):
    self.head += n
    return self

  def e(self):
    # Entangle: Checkout the tape
    self.tape_ref.checkout()
    
    # Create new machine sharing the Tape ref
    new_tm = TM(self.tape_ref)
    new_tm.head = self.head
    
    # Copy handlers
    new_tm._write_handler = self._write_handler
    new_tm._delete_handler = self._delete_handler
    return new_tm

  def dismount(self):
    self.tape_ref.checkin()

  def rightmost(self):
    return self.head >= len(self.tape_ref.data) - 1

  def address(self):
    """For List, address is the integer index."""
    return self.head

  def topology(self):
    return Topology.SEGMENT


class TM_Map(TM):
  """
  Entanglement Accounting Machine specialized for MAPS.
  """
  def __init__(self ,data):
    super().__init__(data)
    self._write_handler = handler_write_map
    # Delete on map is complex (removing key affects order), 
    # disabling default delete for now or need custom handler.
    self._delete_handler = handler_hard_error

  def r(self):
    # Read Value
    key = self.tape_ref.keys_view[self.head]
    return self.tape_ref.data[key]

  def address(self):
    # Address is the Key
    return self.tape_ref.keys_view[self.head]

  def rightmost(self):
    return self.head >= len(self.tape_ref.keys_view) - 1

  def e(self):
    self.tape_ref.checkout()
    new_tm = TM_Map(self.tape_ref)
    new_tm.head = self.head
    return new_tm


class TM_Null(TM_Interface):
  """
  The Mean Machine (No Tape).
  """
  def __init__(self):
    pass

  def r(self): handler_hard_error(self)
  def w(self ,v): handler_hard_error(self)
  def s(self ,n=1): handler_hard_error(self)
  def d(self): handler_hard_error(self)
  def e(self): handler_hard_error(self)
  def address(self): handler_hard_error(self)
  def dismount(self): pass # No tape to checkin
  
  def rightmost(self): handler_hard_error(self)
  def topology(self): return Topology.NULL
  def status(self): return Status.EMPTY


class TM2(TM_Interface):
  """
  Second Order (Status) Machine.
  Wraps a First Order Machine.
  """
  def __init__(self ,data=None ,tm_class=TM):
    if( data and len(data) > 0 ):
      self.tm = tm_class(data)
      self._stat = Status.ACTIVE
    else:
      self.tm = TM_Null()
      self._stat = Status.EMPTY
    
    # Store class for entanglement factories
    self._tm_class = tm_class

  def empty(self):
    return self._stat == Status.EMPTY

  # --- Delegation ---
  
  def r(self): return self.tm.r()
  def w(self ,v): return self.tm.w(v)
  def s(self ,n=1): return self.tm.s(n)
  def d(self): return self.tm.d()
  def address(self): return self.tm.address()
  def dismount(self): self.tm.dismount()
  
  def e(self): 
    if( self.empty() ): return TM2(None)
    
    # Create new wrapper around entangled inner
    new_wrapper = TM2.__new__(TM2)
    new_wrapper.tm = self.tm.e()
    new_wrapper._stat = Status.ACTIVE
    new_wrapper._tm_class = self._tm_class
    return new_wrapper

  def rightmost(self):
    if( self.empty() ): return True
    return self.tm.rightmost()

# ==========================================
# 5. CLI Verification
# ==========================================

def CLI():
  print("--- TTCA Tape Machine Verification ---")

  # 1. List Machine (TM)
  print("\n[Test 1] List Machine (EA)")
  data = ['A' ,'B' ,'C']
  tm2 = TM2(data ,TM)

  if( not tm2.empty() ):
    # Address check
    print(f"Addr {tm2.address()}: {tm2.r()}")
    
    # Entanglement
    tm2_copy = tm2.e()
    tm2.s()
    print(f"Original Moved -> Addr {tm2.address()}: {tm2.r()}")
    print(f"Copy Stayed    -> Addr {tm2_copy.address()}: {tm2_copy.r()}")
    
    # Destructive Check (Should Fail due to 2 owners)
    try:
      tm2.d()
    except RuntimeError as e:
      print(f"Caught Expected EA Error: {e}")
      
    # Dismount copy to allow delete
    tm2_copy.dismount()
    print("Copy dismounted.")
    
    # Now delete should work (deletes right neighbor 'C')
    tm2.d()
    print(f"After Delete: {data}")

  # 2. Map Machine (TM_Map)
  print("\n[Test 2] Map Machine")
  map_data = {'x': 10 ,'y': 20 ,'z': 30}
  tm_map = TM2(map_data ,TM_Map)
  
  if( not tm_map.empty() ):
    print(f"Key {tm_map.address()}: Val {tm_map.r()}")
    tm_map.s()
    print(f"Key {tm_map.address()}: Val {tm_map.r()}")

if __name__ == "__main__":
  CLI()
