# version=2
# > 
# >> code
if a:
    pass
elif b:
    pass
else:
    pass
# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined


# > 
# >> code
def fun(a: bool, b: bool) -> f64:
    if a:
        return 0.0
    elif b:
        return 1.0
    else:
        return 3.0
    return 2.0
# <<

# >> call
fun()# <<


# > case: VM_IfStmt_True
# >> code
def fun(a: i32) -> i32:
    if a > 0:
        return 1
    else:
        return 2
# <<


# >> call
fun(1)# <<


# >> expected
1# <<


# > case: VM_IfStmt_False
# >> code
def fun(a: i32) -> i32:
    if a > 0:
        return 1
    else:
        return 2
# <<


# >> call
fun(0)# <<


# >> expected
2# <<


