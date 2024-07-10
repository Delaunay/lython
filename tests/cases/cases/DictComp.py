# version=2
# > 
# >> code
{a: c for a in b if a > c}# <<

# >> error:: NameError: name 'b' is not defined
# >> error:: NameError: name 'c' is not defined
# >> error:: TypeError: unsupported operand type(s) for Gt: 'None' and 'None'
# >> error:: NameError: name 'c' is not defined


# > 
# >> code
def fun(b: i32):
    return {a: a for a in range(10) if a > b}
# <<

# >> call
fun(2)# <<

