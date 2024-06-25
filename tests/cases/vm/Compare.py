# version=2
# > 
# >> code
def fun(a: i32, b: i32, c: i32, d: i32) -> bool:
    return a < b > c != d
# <<

# >> call
fun()# <<

# > 
# >> code
def fun():
    return a not in b
# <<

# >> call
fun()# <<

# > 
# >> code
def fun():
    return a in b
# <<

# >> call
fun()# <<

# > 
# >> code
def fun():
    return a is b
# <<

# >> call
fun()# <<

# > 
# >> code
def fun():
    return a is not b
# <<

# >> call
fun()# <<

