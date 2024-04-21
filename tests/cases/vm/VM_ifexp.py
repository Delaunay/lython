# >>> case: VM_ifexp_True
# >>> code
def fun(a: i32) -> i32:
    return 1 if a > 0 else 0
# <<<


# >>> call
fun(2)# <<<


# >>> expected
1# <<<


# >>> case: VM_ifexp_False
# >>> code
def fun(a: i32) -> i32:
    return 1 if a > 0 else 0
# <<<


# >>> call
fun(-2)# <<<


# >>> expected
0# <<<


