# version=2
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

