# version=2
# > 
# >> code
def fun():
    return 1, 2, 3
a = fun()
# <<

# > 
# >> code
def fun():
    return 1, 2, 3
a, b, c = fun()
# <<

# > 
# >> code
def fun():
    return 1, 2, 3
a, *b = fun()
# <<

