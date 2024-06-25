# version=2
# > 
# >> code
@e(g, h, i=j)
@f
class a(b, c=d):
    """docstring"""
    pass# <<

# >> error:: NameError: name 'b' is not defined
# >> error:: NameError: name 'd' is not defined
# >> error:: NameError: name 'e' is not defined
# >> error:: e is not callable
# >> error:: NameError: name 'g' is not defined
# >> error:: NameError: name 'h' is not defined
# >> error:: NameError: name 'j' is not defined
# >> error:: NameError: name 'f' is not defined
# > 
# >> code
class Name:
    x: i32 = 0
    y: i32 = 1
    z = 1.2

    def __init__(self):
        self.x = 2
# <<

