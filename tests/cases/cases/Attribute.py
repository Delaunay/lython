# version=2
# > 
# >> code
a.b# <<

# >> call
NameError: name 'a' is not defined# <<


# >> code
class Point:
    x: i32
    y: i32

def fun(a: Point) -> i32:
    return a.x
# <<

# >> call
fun(Point(0, 1))# <<

# >> result:: 0


# > case
# >> code
a.x

# >> SSA
getattr(a, "x")# <<

# > case
# >> code
a.x = b
# <<

# >> SSA
setattr(a, "x", b)# <<
