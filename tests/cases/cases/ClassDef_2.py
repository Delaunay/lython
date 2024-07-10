# version=2
# > 
# >> code
class Name:
    pass

a = Name()
a.x
a.x = 2
# <<

# >> error:: AttributeError: 'Name' has no attribute 'x'
# >> error:: AttributeError: 'Name' has no attribute 'x'
# >> error:: TypeError: expression `a.x` is not compatible with expression `2` of type `i32`
# > 
# >> code
class Custom:
    def __init__(self, a: i32):
        self.a = a

a = Custom(1)
# <<

# > 
# >> code
class Name:
    def __init__(self, x: i32):
        self.x = x

a = Name(2)
a.x
a.x = 4
# <<
