import math


class Position:
    x: float
    y: float

    @property
    def get_x(self) -> float:
        return self.x

    @x.setter
    def set_x(self,  x: float) -> None:
        self.x = math.clamp(x, -1000, 1000)


@abstract
class Shape:
    position: Position

    @virtual
    def perimeter(self) -> float:
        pass

    @virtual
    def area(self) -> float:
        pass


class Circle(Shape):
    radius: float

    def perimeter(self) -> float:
        return math.pi * 2.0 * self.radius

    def area(self) -> float:
        return math.pi * self.radius * self.radius


class Rectangle(Shape):
    width: float
    height: float

    def perimeter(self) -> float:
        return (self.width + self.height) * 2

    def area(self) -> float:
        return self.width * self.height


def main():
    # Static classes
    p = Position(1, 1)
    p.x
    p.y
    p.get_x
    p.set_x(0)

    # virtual
    rect: Shape = Rectangle(10, 20)

    rect.perimeter()
    rect.position
