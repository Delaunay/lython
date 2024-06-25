# version=2
# > 
# >> code
def fun():
    raise a from b
# <<

# >> call
fun()# <<

# > 
# >> code
def fun():
    raise a
# <<

# >> call
fun()# <<

