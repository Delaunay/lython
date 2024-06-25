# version=2
# > 
# >> code
def fun(a: i32) -> i32:    acc = 0
    for a in range(10):
        acc += a
    else:
        pass
    return acc
# <<

# >> call
fun()# <<

# > 
# >> code
def fun():
    for a, (b, c), d in b:
        pass
# <<

# >> call
fun()# <<

