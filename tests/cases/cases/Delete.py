# version=2
# > 
# >> code
del a, b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined

# > 
# >> code
def fun(a: i32, b: i32):
    del a, b
# <<

# >> call
fun(1, 2)# <<

