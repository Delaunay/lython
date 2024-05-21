# >>> case: 0
# >>> code
assert a# <<<

# >>> error
NameError: name 'a' is not defined# <<<

# >>> case: 1
# >>> code
assert a, "b"# <<<

# >>> error
NameError: name 'a' is not defined# <<<

