# version=2
# > 
# >> code
def fun():
    return x
# <<

# >> error:: NameError: name 'x' is not defined
# > 
# >> code
def fun(a: i32) -> i32:
    return a
x = fun(1)
# <<

# > 
# >> code
def fun(a: i32, b: i32 = 2, /, c: i32 = 4, *args, d: i32 = 4, **kwargs) -> i32:
    return a + b + c + d
# <<

# >> call
fun(5, 6, 9, 10, d=7, c=8, e=4)# <<

# >> expected
(i32, i32, i32, i32) -> i32# <<

# > 
# >> code
def fun(a: i32) -> i32:
    return a
x: i32 = fun(1)
# <<

# > 
# >> code
def fun(a: i32) -> i32:
    return a
x = fun(1.0)
# <<

# >> error:: TypeError: expression `fun(1.0)` of type `(f64) -> i32` is not compatible with expression `fun` of type `(i32) -> i32`
# > 
# >> code
def fun(a: i32) -> i32:
    return a
x: f32 = fun(1)
# <<

# >> error:: TypeError: expression `x` of type `f32` is not compatible with expression `fun(1)` of type `i32`
# > 
# >> code
def fun(a: i32, b: f64) -> i32:
    return a
x: i32 = fun(b=1.0, a=1)
# <<

# > 
# >> code
def fun(a: i32 = 1, b: f64 = 1.1) -> i32:
    return a
x: i32 = fun()
# <<

# > 
# >> code
def fun(a: i32, b: f64 = 1.1) -> i32:
    return a
x: i32 = fun()
# <<

# >> error:: TypeError: fun() missing 1 required positional argument: 'a'
