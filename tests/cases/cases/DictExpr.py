# version=2
# > 
# >> code
{a: b, c: d}# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: NameError: name 'c' is not defined
# >> error:: NameError: name 'd' is not defined


# > 
# >> code
def fun():
    return {1: 2, 3: 4}
# <<

# >> call
fun()# <<

