# >>> case: VM_BoolAnd_True
# >>> code
def fun() -> bool:
    return (1 < 2) and (2 < 3)
# <<<


# >>> call
fun()# <<<


# >>> expected
True# <<<


# >>> case: VM_BoolAnd_False
# >>> code
def fun() -> bool:
    return (1 > 2) and (2 > 3)
# <<<


# >>> call
fun()# <<<


# >>> expected
False# <<<


