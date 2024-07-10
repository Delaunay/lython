# version=2
# > 
# >> code
a = b := c# <<

# >> call
NameError: name 'c' is not defined# <<

# > case: VM_NamedExpr
# >> code
def fun(a: i32) -> i32:
    if (b := a + 1) > 0:
        return b
    return 0
# <<


# >> call
fun(0)# <<


# >> expected
1# <<


# > 
# >> code
def fun(c: i32):
    return a := c
# <<

# >> call
fun()# <<

