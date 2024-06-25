# version=2
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

