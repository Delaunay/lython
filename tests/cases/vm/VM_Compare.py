# >>> case: VM_Compare_True
# >>> code
def fun() -> bool:
    return 1 < 2 < 3 < 4 < 5
# <<<


# >>> call
fun()# <<<


# >>> expected
True# <<<


# >>> case: VM_Compare_False
# >>> code
def fun() -> bool:
    return 1 > 2 > 3 > 4 > 5
# <<<


# >>> call
fun()# <<<


# >>> expected
False# <<<


