# version=2
# > 
# >> code
def fun(a: int, c: int):
    with a as b, c as d:
        pass
# <<

# >> call
fun()# <<

# > 
# >> code
def fun():
    with a as b, c as d:
        e = b + d
        e = b + d
        e = b + d
# <<

# >> call
fun()# <<

