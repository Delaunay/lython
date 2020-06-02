

struct Point:
    x: Float
    y: Float


def get_x(p: Point) -> Float:
    return p.x


def set_x(p: Point, x: Float) -> Point:
    p.x = x
    return p


def struct_set_get(v: Float) -> Float:
    p = Point(1.0, 2.0)
    set_x(p, v)
    a = get_x(p)
    return a

