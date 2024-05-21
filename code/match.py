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




# def simple_function(a: b, c: d) -> e:
#     return a + c


# "def test1(p: Float, b):\n"
# "    return sin(1)\n\n"

#     ;
# "import a.b.c\n"
# "import a.b.c as e\n"
# "from a.b.c import f, k\n"
# "from a.b.c import g as h, i as j\n\n"

# "struct Point:\n"
# "    x: Float\n"
# "    y: Float\n"
# "\n"

# "def test1(p: Float) -> Float:\n"
# "    return sin(1)\n\n"

# "def test3(p: Float) -> Float:\n"
# "    return sin(max(sin(p * 2), sin(p + 1)))\n\n"

# "def test2(p: Float) -> Float:\n"
# "    return sin(max(2, 3) / 3 * p)\n\n"

# "def get_x(p: Point) -> Float:\n"
# "    return p.x\n\n"

# "def set_x(p: Point, x: Float) -> Point:\n"
# "    p.x = x\n"
# "    return p\n\n"

# "def struct_set_get(v: Float) -> Float:\n"
# "    p = Point(1.0, 2.0)\n"
# "    set_x(p, v)\n"
# "    a = get_x(p)\n"
# "    return a\n\n"

# "def call_import() -> Float:\n"
# "    return k(1.0, 2.0)\n\n";

# "def function2(test: double, test) -> double:\n"
# "    \"\"\"This is a docstring\"\"\"\n"
# "    return add(1, 1)\n\n"

# "def function3(test: int, test) -> e:\n"
# "    return add(1, 1)\n\n"

# "struct Object:\n"
# "    \"\"\"This is a docstring\"\"\"\n"
# "    a: Type\n";
