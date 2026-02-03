#!/usr/bin/env python3
import copy

class TapeMachine:
    """
    TTCA Tape Machine Implementation.
    Adapts List, Map, and Set to a common Tape Interface.
    """
    def __init__(self, tape_ref, path=None, point=0, iterator=None):
        self.tape = tape_ref        # The 'Memory' (Shared)
        self.path = path or []      # The 'Stack' (Hierarchy context)
        
        # 'point' is the Address. 
        # For Lists: Integer Index
        # For Maps:  Key (Symbol)
        # For Sets:  The Item itself
        self.point = point
        
        # For Maps/Sets, we need an iterator to support 'Step' (s)
        self._iter = iterator

    # --- (e) Entangle ---
    def e(self):
        """
        Entangled Copy. 
        Returns a new head sharing the same tape memory.
        """
        # We must clone the iterator state if possible, 
        # though Python iterators are hard to clone. 
        # We usually restart iteration or assume random access.
        return TapeMachine(self.tape, self.path, self.point)

    # --- (r) Read ---
    def r(self):
        """Reads the cell under the head."""
        container = self._resolve_container()
        
        if isinstance(container, list):
            if 0 <= self.point < len(container): 
                return container[self.point]
                
        elif isinstance(container, dict):
            return container.get(self.point, None)
            
        elif isinstance(container, set):
            # In a set, if we are 'at' a point, the value IS the point.
            # But we must verify it still exists.
            return self.point if self.point in container else None
            
        return None

    # --- (w) Write ---
    def w(self, value):
        """Writes to the cell under the head."""
        container = self._resolve_container()
        
        if isinstance(container, list):
            container[self.point] = value
            
        elif isinstance(container, dict):
            container[self.point] = value
            
        elif isinstance(container, set):
            raise TypeError("Cannot 'Write' to a Set cell. Use 'd' (Delete) and 'a' (Add).")

    # --- (s) Step ---
    def s(self, direction=1):
        """
        Move relative to current position.
        Direction +1 = Next, -1 = Previous.
        """
        container = self._resolve_container()

        if isinstance(container, list):
            self.point += direction
            
        elif isinstance(container, (dict, set)):
            # Maps/Sets require iteration to step.
            # This is expensive (O(N)) unless we maintain an active iterator.
            # Simplified Logic:
            try:
                # In a real engine, we'd cache the list of keys
                keys = list(container.keys()) if isinstance(container, dict) else list(container)
                
                # Find current index
                try:
                    current_idx = keys.index(self.point)
                    next_idx = current_idx + direction
                    if 0 <= next_idx < len(keys):
                        self.point = keys[next_idx]
                except ValueError:
                    # Current point no longer in set/map, reset to start
                    if keys: self.point = keys[0]
            except:
                pass
        return self

    # --- (m) Move ---
    def m(self, address):
        """
        Absolute jump to an address.
        List: index (int), Map: key (symbol), Set: member (symbol).
        """
        self.point = address
        return self

    # --- (a) Allocate / Add ---
    def a(self, value, key=None):
        """
        Appends or Inserts.
        List: Append value.
        Map:  Insert key:value.
        Set:  Add value.
        """
        container = self._resolve_container()
        
        if isinstance(container, list):
            container.append(value)
            
        elif isinstance(container, dict):
            if key is None: raise ValueError("Map allocation requires key")
            container[key] = value
            
        elif isinstance(container, set):
            container.add(value)

    # --- Hierarchy Navigation (Enter/Exit) ---
    def enter(self):
        """Descends into the current cell."""
        current_val = self.r()
        
        # Valid container?
        if isinstance(current_val, (list, dict, set)):
            # Push context
            new_path = self.path + [(self._resolve_container(), self.point)]
            
            # Determine starting point for new container
            start_point = 0
            if isinstance(current_val, (dict, set)) and len(current_val) > 0:
                # Start at the first key/member
                start_point = next(iter(current_val))
                
            return TapeMachine(self.tape, new_path, start_point)
        return None

    def exit(self):
        """Ascends to parent."""
        if not self.path: return None
        
        # Pop context
        parent_container, parent_point = self.path[-1]
        new_path = self.path[:-1]
        
        return TapeMachine(self.tape, new_path, parent_point)

    # --- Internal ---
    def _resolve_container(self):
        """Drills down the path stack to find current container."""
        curr = self.tape
        for container, index in self.path:
            if isinstance(container, list): curr = container[index]
            elif isinstance(container, dict): curr = container[index]
            # Set traversal in path stack implies we 'entered' a member 
            # (which must be a container itself)
        return curr
