# version=2
# > 
# >> code
a[b]# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined

# > 
# >> code
def fun(a: list[i32]) -> i32:
    return a[0]
# <<

# >> call
fun([1, 2])# <<

