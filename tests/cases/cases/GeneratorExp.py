# version=2
# > 
# >> code
(a for a in b if a > c)# <<

# >> error:: NameError: name 'b' is not defined
# >> error:: NameError: name 'c' is not defined
# >> error:: TypeError: unsupported operand type(s) for Gt: 'None' and 'None'


# > 
# >> code
def fun(b: i32) -> list[i32]:
    return (a for a in range(10) if a > b)
# <<

# >> call
fun(2)# <<

