# version=2
# > 
# >> code
global a# <<

# >> error:: NameError: name 'a' is not defined

# > 
# >> code
a: i32 = 1
def fun():
    global a
    a += 1
    return a
# <<

# >> call
fun()# <<

