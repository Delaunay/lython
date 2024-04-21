# >>> case: VM_assign
# >>> code
def fun(a: i32) -> i32:
    b = 3
    b = a * b
    return b
# <<<


# >>> call
fun(2)# <<<


# >>> expected
6# <<<


