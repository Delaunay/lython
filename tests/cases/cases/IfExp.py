# version=2
# > 
# >> code
a = c if True else d# <<

# >> error:: NameError: name 'c' is not defined
# >> error:: NameError: name 'd' is not defined


# > case: VM_ifexp_True
# >> code
def fun(a: i32) -> i32:
    return 1 if a > 0 else 0
# <<


# >> call
fun(2)# <<


# >> expected
1# <<


# > case: VM_ifexp_False
# >> code
def fun(a: i32) -> i32:
    return 1 if a > 0 else 0
# <<


# >> call
fun(-2)# <<


# >> expected
0# <<


# > 
# >> code
def fun(a: i32, b: i32) -> i32:
    return a if True else b
# <<

# >> call
fun(1, 2)# <<
