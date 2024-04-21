# >>> case: VM_pass
# >>> code
def fun(a: i32) -> i32:
    while False:
        pass
    return 0
# <<<


# >>> call
fun(0)# <<<


# >>> expected
0# <<<


