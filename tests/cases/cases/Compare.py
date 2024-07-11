# version=2
# > 
# >> code
a < b > c != d# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for Lt: 'None' and 'None'
# >> error:: NameError: name 'c' is not defined
# >> error:: TypeError: unsupported operand type(s) for Gt: 'None' and 'None'
# >> error:: NameError: name 'd' is not defined
# >> error:: TypeError: unsupported operand type(s) for NotEq: 'None' and 'None'
# > 
# >> code
a not in b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for NotIn: 'None' and 'None'
# > 
# >> code
a in b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for In: 'None' and 'None'
# > 
# >> code
a is b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for Is: 'None' and 'None'
# > 
# >> code
a is not b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for IsNot: 'None' and 'None'


# > case: VM_Compare_True
# >> code
def fun() -> bool:
    return 1 < 2 < 3 < 4 < 5
# <<

# >> call
fun()# <<

# >> results
True# <<


# >> case: VM_Compare_False
# >> code
def fun() -> bool:
    return 1 > 2 > 3 > 4 > 5
# <<

# >> call
fun()# <<

# >> results
False# <<


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


# > Compare SSA
# >> code
e = a < b < c < d# <<

# >> SSA
LY_0 = a < b
LY_1 = b < c
LY_2 = c < d
LY_3 = __and__(LY_0, LY_1)
e = __and__(LY_3, LY_2)# <<