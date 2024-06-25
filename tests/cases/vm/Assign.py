# version=2
# > 
# >> code
def fun(b: i32) -> i32:
    a = b
    return a
# <<

# >> call
fun(1)# <<

# > 
# >> code
def fun(c: Tuple[i32, i32]) -> i32:
    a, b = c
    return a
# <<

# >> call
fun((1, 2))# <<

# > 
# >> code
def fun() -> i32:
    a = 1
    return a
# <<

# >> call
fun()# <<

# >> expected
i32# <<


# > 
# >> code
def fun():
    a = 1.0
    return a
# <<

# >> call
fun()# <<

# >> expected
f64# <<


