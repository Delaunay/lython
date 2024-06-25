# version=2
# > 
# >> code
def fun(a: i32, b: i32) -> i32:
    a += b
    return a
# <<

# >> call
fun(2, 1)# <<

# > 
# >> code
def fun(a: i32, b: i32) -> i32:
    a -= b
    return a
# <<

# >> call
fun(2, 1)# <<

