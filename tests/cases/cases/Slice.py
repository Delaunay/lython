# version=2
# > 
# >> code
a[b:c:d]# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: NameError: name 'c' is not defined
# >> error:: NameError: name 'd' is not defined


# > 
# >> code
def fun(a: list[i32], s: i32, step: i32, e: i32):
    return a[s:step:e]
# <<

# >> call
fun([1, 2, 3, 4, 5, 6], 0, 2 ,4)# <<

