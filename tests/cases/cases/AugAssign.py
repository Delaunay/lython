# version=2
# > 
# >> code
a += b# <<

# >> SSA
LY_0:  = a + b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for Add: 'None' and 'None'
# > 
# >> code
a -= b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for Sub: 'None' and 'None'

# >> SSA
LY_0:  = a - b# <<




# > case: VM_Aug_Add_i32
# >> code
def fun(a: i32) -> i32:
    a += 1
    return a
# <<

# >> call
fun(0)# <<

# >> result
1# <<


# > 
# >> code
def fun(a: i32, b: i32) -> i32:
    a += b
    return a
# <<

# >> call
fun(2, 1)# <<

# > 
# >> code
def fun(a: i32, b: i32) -> i32:
    a -= b
    return a
# <<

# >> call
fun(2, 1)# <<

