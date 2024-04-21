# >>> case: VM_AnnAssign
# >>> code
def fun(a: i32) -> i32:
    b: i32 = 3
    b: i32 = a * b
    return b
# <<<


# >>> call
fun(2)# <<<


# >>> expected
6# <<<


