# version=2
# > 
# >> code
a# <<

# >> call
NameError: name 'a' is not defined# <<

# > 
# >> code
def fun(a: i32) -> i32:
    return a
# <<

# >> call
fun(1)# <<

