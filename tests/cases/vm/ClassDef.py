# version=2
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

