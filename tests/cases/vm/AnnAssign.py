# version=2
# > 
# >> code
def fun() -> bool:
    a: bool = True
    return a
# <<

# >> call
fun()# <<

# > 
# >> code
def fun() -> i32:
    a: i32 = 1
    return a
# <<

# >> call
fun()# <<

# > 
# >> code
def fun() -> f64:
    a: f64 = 2.0
    return a
# <<

# >> call
fun()# <<

