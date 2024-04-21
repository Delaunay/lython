# >>> case: VM_ClassDef
# >>> code
class Point:
    def __init__(self, x: f64, y: f64):
        self.x = x
        self.y = y

# <<<


# >>> call
Point(1.0, 2.0)# <<<


# >>> expected
(x=1.0, y=2.0)# <<<


# >>> case: VM_ClassDef_2
# >>> code
class Point:
    def __init__(self, x: f64, y: f64):
        self.x = x
        self.y = y

def fun(p: Point) -> f64:
    return p.x + p.y
# <<<


# >>> call
fun(Point(1.0, 2.0))# <<<


# >>> expected
3.0# <<<


