# version=2
# > 
# >> code
*a# <<

# >> call
NameError: name 'a' is not defined# <<


# > 
# >> code
def fun(a: list[i32]) -> list[i32]:
    return *a
# <<

# >> call
fun([1, 2])# <<

