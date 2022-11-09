from __future__ import annotations
from typing import TypeVar, Tuple
from typing import NewType


class Product(type):
  def __and__(self, other):
    return Tuple[self, other]

  def __rand__(self, other):
    return "Hello"

class Node(metaclass=Product):
  pass


Node = (
    (Scalar := NewType("Scalar", float | int | str)) |
    (Multiply := NewType("Multiply",  Node & Node)) |
    (Add :=  NewType("Add", Node & Node))
)

print(Node)
print(Multiply)

def print_node(node):
  match node:
    case Scalar(val):
        print('Scalar')
    case Multiply(lhs, rhs):
        print('Multiply')
    case Add(lhs, rhs):
        print('Add')


print(tuple[int, int](1, 1))

s = Scalar(1)
m = Multiply(s, s)
a = Add(s, s)

print_node(s)
print_node(m)
print_node(a)