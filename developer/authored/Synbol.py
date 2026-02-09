class Symbol(set):
    """
    Epimetheus Symbol.
    
    Properties:
      1. Identity: Each instance is unique (based on memory address).
      2. Container: It is a Set. Differentiation puts child symbols inside parents.
      3. Hashable: It uses id(self), allowing symbols to contain symbols.
    """
    
    def __hash__(self):
        # The distinctness of the symbol is its memory address (Process Local ID)
        return id(self)

    def __eq__(self, other):
        # Symbols are only equal if they are the same object
        return self is other

    def __repr__(self):
        # Hex address is the standard Python way to represent opaque identity
        return f"<Symbol {id(self):x}>"

# --- The Factory ---

class Epimetheus:
    @staticmethod
    def symbol():
        """Mints a new, unique Symbol."""
        return Symbol()

    @staticmethod
    def differentiate(parent_symbol):
        """
        Creates a new symbol that is a differentiation of the parent.
        The child is added to the parent's set.
        """
        child = Symbol()
        parent_symbol.add(child)
        return child

# --- Usage Example ---

def test_symbols():
    # 1. Mint a Root Symbol
    color = Epimetheus.symbol()
    print(f"Root: {color}")

    # 2. Differentiate
    red = Epimetheus.differentiate(color)
    blue = Epimetheus.differentiate(color)
    
    print(f"Red:  {red}")
    print(f"Blue: {blue}")

    # 3. Verify Structure
    print(f"Color contains Red?  {red in color}")  # True
    print(f"Color contains Blue? {blue in color}") # True
    print(f"Red is Blue?         {red == blue}")   # False
    
    # 4. Nested Differentiation (Hierarchical)
    dark_red = Epimetheus.differentiate(red)
    print(f"Red contains DarkRed? {dark_red in red}") # True
    print(f"Color contains DarkRed? {dark_red in color}") # False (Direct containment only)

if __name__ == "__main__":
    test_symbols()
