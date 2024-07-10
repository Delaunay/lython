# version=2
# > 
# >> code
+ a# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: TypeError: unsupported operand type(s) for UAdd: 'None' and 'None'
# > 
# >> code
- a# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: TypeError: unsupported operand type(s) for USub: 'None' and 'None'
# > 
# >> code
~ a# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: TypeError: unsupported operand type(s) for Invert: 'None' and 'None'
# > 
# >> code
! a# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: TypeError: unsupported operand type(s) for Not: 'None' and 'None'


# > 
# >> code
def fun(a: i32) -> i32:
    return + a
# <<

# >> call
fun()# <<

# > 
# >> code
def fun(a: i32) -> i32:
    return - a
# <<

# >> call
fun()# <<

# > 
# >> code
def fun(a: i32) -> i32:
    return ~ a
# <<

# >> call
fun()# <<

# > 
# >> code
def fun(a: i32) -> i32:
    return ! a
# <<

# >> call
fun()# <<


# > case: VM_assert_True
# >> code
def fun(a: i32) -> i32:
    return - a
# <<


# >> call
fun(-1)# <<


# >> expected
1# <<


