# version=2
# > 
# >> code
with a as b, c as d:
    pass
# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'c' is not defined
# > 
# >> code
with a as b, c as d:
    e = b + d
    e = b + d
    e = b + d
# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'c' is not defined


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
