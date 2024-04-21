# >>> case: VM_IfStmt_True
# >>> code
def fun(a: i32) -> i32:
    if a > 0:
        return 1
    else:
        return 2
# <<<


# >>> call
fun(1)# <<<


# >>> expected
1# <<<


# >>> case: VM_IfStmt_False
# >>> code
def fun(a: i32) -> i32:
    if a > 0:
        return 1
    else:
        return 2
# <<<


# >>> call
fun(0)# <<<


# >>> expected
2# <<<


