# version=2
# > 
# >> code
@j
def a(b, c=d, /, e=f, *g, h=i, **j) -> bool:
    """docstring"""
    return True# <<

# >> error:: NameError: name 'd' is not defined
# >> error:: NameError: name 'g' is not defined
# >> error:: NameError: name 'j' is not defined
# > 
# >> code
@j
def a(b, c=d, *e, f=g, **h) -> bool:
    """docstring"""
    return True# <<

# >> error:: NameError: name 'd' is not defined
# >> error:: NameError: name 'g' is not defined
# >> error:: NameError: name 'j' is not defined
# > 
# >> code
@j(l, m, c=n)
@k
def a(b: bool, d: bool = True):
    pass# <<

# >> error:: NameError: name 'j' is not defined
# >> error:: j is not callable
# >> error:: NameError: name 'l' is not defined
# >> error:: NameError: name 'm' is not defined
# >> error:: NameError: name 'n' is not defined
# >> error:: NameError: name 'k' is not defined



# > 
# >> code
@j
def a(b, c=d, *e, f=g, **h) -> bool:
    """docstring"""
    return True
# <<

# >> call
fun()# <<

# > 
# >> code
@j(l, m, c=n)
@k
def a(b: bool, d: bool = True):
    pass
# <<

# >> call
fun()# <<
