#!/usr/bin/env python3
from SymbolSpace import SymbolSpace
from Binder import Binder
from DiscreteFunction import DiscreteFunction
from Namespace import DifferentiatedSymbol ,OrderedNamespace

def example_queries():
  print("--- Epimetheus Architecture Example ---")

  # 1. System Initialization
  binder = Binder()
  graph = DiscreteFunction()

  # 2. Ontology Definition (The Factories)
  Frame = DifferentiatedSymbol("FrameMaterial")
  Price = OrderedNamespace("Price")

  # 3. Data Ingestion (Transient Objects)
  data_source = [
    ("Bike_A" ,"Carbon" ,3500),
    ("Bike_B" ,"Steel"  ,800),
    ("Bike_C" ,"Alum"   ,1200),
    ("Bike_D" ,"Carbon" ,1500),
  ]

  print(f"Ingesting {len(data_source)} items...")
  
  # Keep references to prevent GC during ingestion for this example
  start_objects = [] 

  for label, material, cost in data_source:
    # A. Create the Python Object
    obj = type("Bike", (), {"label": label})()
    start_objects.append(obj)
    
    # B. Bind: Object -> Symbol
    sym_bike = binder.get_symbol(obj)
    
    # C. Describe: Symbol -> Properties
    graph.set(sym_bike ,Frame(material))
    graph.set(sym_bike ,Price(cost))

  # ---------------------------------------------------------
  # Example 1: Exact Query ("Find Carbon Frames")
  # ---------------------------------------------------------
  print("\n[Example 1] Exact Match: Frame('Carbon')")
  
  # We ask the Factory for the symbol representing 'Carbon'
  sym_carbon = Frame("Carbon")
  
  # We ask the Graph for entities with that symbol
  results = graph.find(sym_carbon)
  print(f" -> Found {len(results)} bikes with Carbon frames.")

  # ---------------------------------------------------------
  # Example 2: Range Query ("Find Price 1000..2000")
  # ---------------------------------------------------------
  print("\n[Example 2] Range Match: Price(1000..2000)")

  # Step A: Ask Namespace for symbols in range
  # The OrderedNamespace uses bisect to find symbols efficiently
  price_symbols = Price.find_range(1000 ,2000)
  print(f" -> Namespace identified {len(price_symbols)} relevant price points.")

  # Step B: Ask Graph for objects having ANY of those symbols
  matches = set()
  for p_sym in price_symbols:
    found = graph.find(p_sym)
    matches.update(found)

  print(f" -> Graph resolved {len(matches)} bikes in price range.")

  # ---------------------------------------------------------
  # Example 3: Hierarchy Query ("Find Priced Things")
  # ---------------------------------------------------------
  print("\n[Example 3] Hierarchy Match: Has Price")
  
  # We query the Root Symbol of the Price namespace.
  # This works because the Graph automatically 'posts' up to the parent.
  # FIXED: Changed .root to .root_symbol to match Namespace.py
  all_priced = graph.find(Price.root_symbol)
  print(f" -> Found {len(all_priced)} objects that have a price.")

def CLI():
  example_queries()

if __name__ == "__main__":
  CLI()
