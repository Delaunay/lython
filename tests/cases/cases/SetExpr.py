# version=2
# > 
# >> code
{a, b}# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined


# > 
# >> code
def fun():
    return {1, 2}
# <<

# >> call
fun()# <<

