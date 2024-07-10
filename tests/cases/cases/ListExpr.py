# version=2
# > 
# >> code
[a, b, c]# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: NameError: name 'c' is not defined

# > 
# >> code
def fun(a: i32) -> list[i32]:
    return [a, a, a]
# <<

# >> call
fun(1)# <<

