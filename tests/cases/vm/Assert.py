# version=2
# > 
# >> code
def fun(a: int):
    assert a
# <<

# >> call
fun()# <<

# > 
# >> code
def fun(a: int):
    assert a, "b"# <<

# >> call
fun()# <<

