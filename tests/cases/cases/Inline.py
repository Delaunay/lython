# version=2
# > 
# >> code
a = 2; b = c; d = e# <<

# >> error:: NameError: name 'c' is not defined
# >> error:: NameError: name 'e' is not defined


# > case: VM_inline_stmt
# >> code
def fun(a: i32) -> i32:
    a += 1; return a
# <<


# >> call
fun(0)# <<


# >> expected
1# <<

