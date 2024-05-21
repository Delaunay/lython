# >>> case: 0
# >>> code
def fun():
    return x
# <<<

# >>> call
NameError: name 'x' is not defined# <<<

# >>> case: 1
# >>> code
def fun(a: i32) -> i32:
    return a
x = fun(1)
# <<<

# >>> case: 2
# >>> code
def fun(a: i32) -> i32:
    return a
x: i32 = fun(1)
# <<<

# >>> case: 3
# >>> code
def fun(a: i32) -> i32:
    return a
x = fun(1.0)
# <<<

# >>> call
TypeError: expression `fun(1.0)` of type `(f64) -> i32` is not compatible with expression `fun` of type `(i32) -> i32`# <<<

# >>> case: 4
# >>> code
def fun(a: i32) -> i32:
    return a
x: f32 = fun(1)
# <<<

# >>> call
TypeError: expression `x` of type `f32` is not compatible with expression `fun(1)` of type `i32`# <<<

# >>> case: 5
# >>> code
def fun(a: i32, b: f64) -> i32:
    return a
x: i32 = fun(b=1.0, a=1)
# <<<

# >>> case: 6
# >>> code
def fun(a: i32 = 1, b: f64 = 1.1) -> i32:
    return a
x: i32 = fun()
# <<<

# >>> case: 7
# >>> code
def fun(a: i32, b: f64 = 1.1) -> i32:
    return a
x: i32 = fun()
# <<<

