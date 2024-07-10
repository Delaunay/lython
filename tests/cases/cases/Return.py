# version=2
# > 
# >> code
return a# <<

# >> call
NameError: name 'a' is not defined# <<

# > 
# >> code
return 1, 2# <<

# > 
# >> code
return a + b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# > 
# >> code
return p.x + p.y# <<

# >> error:: NameError: name 'p' is not defined
# >> error:: NameError: name 'p' is not defined


# > 
# >> code
def fun(a: i32) -> i32:
    return a
# <<

# >> call
fun()# <<

# > 
# >> code
def fun() -> Tuple[i32, i32]:
    return 1, 2
# <<

# >> call
fun()# <<

# > 
# >> code
def fun() -> i32:
    return 1 + 2
# <<

# >> call
fun()# <<

