# >>> case: 0
# >>> code
def fun(a: i32) -> i32:
    return a
# <<<


# >>> call
fun(1)# <<<


# >>> expected
1# <<<


# >>> case: 1
# >>> code
def fun(a: i32) -> i32:
    return a + 1
# <<<


# >>> call
fun(1)# <<<


# >>> expected
2# <<<


# >>> case: 2
# >>> code
def fun(a: i32, b: i32) -> i32:
    b += 1
    if a == 0:
        return b
    return fun(a - 1, b)
# <<<


# >>> call
fun(10, 0)# <<<


# >>> expected
11# <<<


# >>> case: 3
# >>> code
def fun(a: i32) -> i32:
    if a == 0:
        return 0
    return fun(a - 1) + 1
# <<<


# >>> call
fun(10)# <<<


# >>> expected
10# <<<


