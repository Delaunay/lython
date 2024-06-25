# version=2
# > 
# >> code
def fun(a: i32) -> Tuple[i32, i32, i32]:
    return a, a, a
# <<

# >> call
fun(1)# <<

# > 
# >> code
def fun(v: Tuple[i32, Tuple[i32, i32], i32]):
    a, (b, c), d = v
    return b
# <<

# >> call
fun((1, (2, 3), 4))# <<

# > 
# >> code
def fun(v: Tuple[i32, i32]) -> i32:
    d, e, f = v
    a, b, c = d, e, f
    return b
# <<

# >> call
fun((1, 2, 3))# <<

