# version=2
# > 
# >> code
def fun(a: i32) -> i32:
    return a
# <<

# >> call
fun()# <<

# > 
# >> code
def fun() -> Tuple[i32, i32]:
    return 1, 2
# <<

# >> call
fun()# <<

# > 
# >> code
def fun() -> i32:
    return 1 + 2
# <<

# >> call
fun()# <<

