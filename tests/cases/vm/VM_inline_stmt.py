# >>> case: VM_inline_stmt
# >>> code
def fun(a: i32) -> i32:
    a += 1; return a
# <<<


# >>> call
fun(0)# <<<


# >>> expected
1# <<<


