# version=2
# > 
# >> code
nonlocal a# <<

# >> error:: NameError: name 'a' is not defined


# > 
# >> code
def fun():
    a: list = []
    def _():
       nonlocal a
       a.append(1)
    _()
    return a
# <<

# >> call
fun()# <<

