# > case: VM_ClassDef
# >> code
class Point:
    def __init__(self, x: f64, y: f64):
        self.x = x
        self.y = y

# <<


# >> call
Point(1.0, 2.0)# <<


# >> result
(x=1.0, y=2.0)# <<


# >> case: VM_ClassDef_2
# >> code
class Point:
    def __init__(self, x: f64, y: f64):
        self.x = x
        self.y = y

def fun(p: Point) -> f64:
    return p.x + p.y
# <<


# >> call
fun(Point(1.0, 2.0))# <<


# >> result
3.0# <<


# > 
# >> code
@e(g, h, i=j)
@f
class a(b, c=d):
    """docstring"""
    pass# <<

# >> call
fun()# <<

# > 
# >> code
class Name:
    x: i32 = 0
    y: i32 = 1
    z = 1.2

    def __init__(self):
        self.x = 2
def fun():
    return Name().x
# <<

# >> call
fun()# <<
