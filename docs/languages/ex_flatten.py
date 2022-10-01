#
#   Note: Generate functions here are written with a '_' as the namespace separator
#   inside lython the true namespace separator is '.' which prevents users from
#   giving function names that could clash with our generated functions
#

from __future__ import annotations
import math
from typing import Callable

from lython import *

#
# Runtime reflection can be removed at link time
# it will require a small runtime overhead
#
#   type(obj) => rt_object.lookup(obj) => Position_reflection
#
#   this require runtime reflection if the type of obj is not known at compile time
#   (means obj is a polymorphic type)
#
#   getattr(obj, "x")
#
#

# Garbage collection
#
# lython use stack memory by default.
# You can allocated object on the heap using malloc or the root GC
# when allocating with a GC you will need to specify a parent
# the parent is an object that can be both allocated on the stack or on the heap
# when the parent object is deleted all its children gets cleaned up as well
#
# note: that does not prevent user from using references of deleted object
# we could mitigate this by allowing more than one parent
# and only delete when all the parents are removed (essentially making the object ref counted)
# but we would expect most objects to have a single parent, so the ref counting could be fairly limited
#
# NOTE: we need to support multi device memory management
# GPU & CPU
#

# With runtime-reflection
# Generate data to reply to runtime type query
Position_reflection = Object(
    Property('__name__', 'Position'),
    Property('x', float),
    Property('y', float),
)

Shape_reflection = None
Circle_reflection  = None
Rectangle_reflection = None

Module_Type = [
    Position_reflection,
    Shape_reflection,
    Circle_reflection,
    Rectangle_reflection,
]


# We need runtime typeinfo for this
# I would rather not have a property here
# maybe I can have a global RTTI struct that map
# allocated object to their type
class Position:
    x: float
    y: float


def Position_get_x(self: Position) -> float:
    return self.x

def Position_set_x(self: Position, x: float) -> None:
    self.x = math.clamp(x, -1000, 1000)


def Position___init__(x: float, y: float) -> Position:
    obj = Object(x, y)

    # Runtime reflection type info
    # this will mark obj with its type, doing it this way
    # enable us to chose at link time if we want RR or not
    # Link can remove all the RR stuff
    rr_ti(obj, Position_reflection)
    return obj


class Shape_vtable:
    perimeter: Callable[[Shape], float]
    area: Callable[[Shape], float]


Shape_position_address = sizeof(Shape_vtable)


class Shape:
    vtable: Shape_vtable
    position: Position


def Shape_perimeter(self: Shape) -> float:
    vtable: Shape_vtable = self._Shape_vtable
    return vtable.perimteer(self)


def Shape_area(self: Shape) -> float:
    vtable: Shape_vtable = self._Shape_vtable
    return vtable.perimteer(self)


class Circle:
    vtable: Shape_vtable
    position: Position
    radius: float

def Circle_perimeter(self: Shape) -> float:
    return math.pi * 2.0 * self.radius


#
# Note: we could do de-virtualization here by exploiting the
# type knowledge and use `Circle_area` directly instead of `Shape_area`
#
def Circle_area(self: Shape) -> float:
    return math.pi * self.radius * self.radius


# Simple array storing the addresses of the implementation
Circle_vtable = Shape_vtable(
    Circle_perimeter,
    Circle_area
)

#
# Here Object is a generic value that represent all the attributes as an array
# or simply a contiguous chunk of memory
#
def Circle___init__(position: Position, radius: float) -> Circle:
    obj = Object(Circle_vtable, position, radius)
    return obj


class Rectangle(Shape):
    vtable: Shape_vtable
    position: Position
    width: float
    height: float


def Rectangle_perimeter(self: Shape) -> float:
    return (self.width + self.height) * 2


def Rectangle_area(self: Shape) -> float:
    return self.width * self.height


Rectangle_vtable = Shape_vtable(
    Rectangle_perimeter,
    Rectangle_area
)

def Rectangle___init__(position: Position, width: float, height: float) -> Circle:
    obj = Object(Rectangle_vtable, position, width, height)
    return obj


def main():
    #
    # Note: constant folding here is disabled
    # else none of this code would endup being generated
    #
    p = Position___init__(1, 1)
    p[0]
    p[sizeof(float)]
    Position_get_x(p)
    Position_set_x(p, 0)

    # Type gets overriden from Rectangle to Shape
    # this means we have to call the Shape version with the vtable
    # else it would call the regular perimeter function without the
    # dynamic dispatch
    rect: Shape = Rectangle___init__(10, 20)
    Shape_perimeter(rect)
    rect[Shape_position_address]
