# >>> case: 0
# >>> code
def fun():
    return 1, 2, 3
a = fun()
# <<<

# >>> case: 1
# >>> code
def fun():
    return 1, 2, 3
a, b, c = fun()
# <<<

# >>> case: 2
# >>> code
def fun():
    return 1, 2, 3
a, *b = fun()
# <<<

