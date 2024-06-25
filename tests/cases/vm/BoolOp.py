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

