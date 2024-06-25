# version=2
# > 
# >> code
def fun(b: i32) -> list[i32]:
    return [a for a in range(10) if a > b]
# <<

# >> call
fun(2)# <<

