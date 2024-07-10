# version=2
# > Simple Addition
# >> code
a + b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# > 
# >> code
a - b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# > 
# >> code
a * b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# > 
# >> code
a << b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# > 
# >> code
a ^ b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined

# > case: VM_BinOp_Add_i32
# >> code
def fun(a: i32) -> i32:
    return a + 1
# <<

# >> call
fun(1)# <<<

# >> expected
2# <<


# > case: VM_BoolAnd_True
# >> code
def fun() -> bool:
    return (1 < 2) and (2 < 3)
# <<

# >> call
fun()# <<

# >> expected
True# <<

# > case: VM_BoolAnd_False
# >> code
def fun() -> bool:
    return (1 > 2) and (2 > 3)
# <<

# >> call
fun()# <<

# >> expected
False# <<



# version=2
# > 
# >> code
def fun(a: bool, b: bool) -> bool:
    return a and b
# <<

# >> call
fun(True, False)# <<

# > 
# >> code
def fun(a: bool, b: bool) -> bool:
    return a or b
# <<

# >> call
fun(True, False)# <<

# > 
# >> code
def fun(a: bool, b: bool, c: bool) -> bool:
    return a or b or c
# <<

# >> call
fun(True, False, False)# <<

# > 
# >> code
def fun(a: f64, b: f64) -> f64:
    return a + b
# <<

# >> call
fun()# <<

# > 
# >> code
def fun(a: f64, b: f64) -> f64:
    return a - b
# <<

# >> call
fun()# <<

# > 
# >> code
def fun(a: f64, b: f64) -> f64:
    return a * b
# <<

# >> call
fun()# <<

# > 
# >> code
def fun(a: i32, b: i32) -> i32:
    return a << b
# <<

# >> call
fun()# <<

# > 
# >> code
def fun(a: i32, b: i32) -> i32:
    return a ^ b
# <<

# >> call
fun()# <<

