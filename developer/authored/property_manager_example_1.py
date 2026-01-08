#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2 -*-


"""
property_manager_example_1.py

Demonstrate RT identity-based PropertyManager.

Run:
  PYTHONPATH=. python3 property_manager_example_1.py
"""

from rt_property_manager import PropertyManager


class WidgetFactory:
  def __call__(self ,x):
    return Widget(x)


class Widget:
  def __init__(self ,x):
    self.x = x

  def add(self ,y):
    return self.x + y


def main():
  pm = PropertyManager()

  # Declare semantic set "WidgetFactories"
  set_widget_factories_id = pm.declare_set(["semantic" ,"WidgetFactories"] ,"Factories that produce Widgets")

  wf = WidgetFactory()
  pm.add_to_set(wf ,set_widget_factories_id)

  # Declare a property "printer" (intended to attach to methods)
  prop_printer_id = pm.declare_property(["semantic" ,"printer"] ,"Callable that prints the value of an instance")

  # Attach property to Widget.add method object (unbound function attribute on class)
  pm.set(Widget.add ,prop_printer_id ,lambda inst: print(f"Widget(x={inst.x})"))

  w = wf(7)

  # Semantic check: require that the provenance factory is in the WidgetFactories set
  # (In this example we didn't record provenance; we'd do that via an explicit call later.)

  # Call printer property on the method we care about
  printer = pm.get(Widget.add ,prop_printer_id)
  printer(w)

  # Reverse lookup: which subjects have 'printer'?
  subject_ids = pm.subjects_with(prop_printer_id)
  print("subjects_with(printer):" ,len(subject_ids))


if __name__ == "__main__":
  main()
