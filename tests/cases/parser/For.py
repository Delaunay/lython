# >>> case: 0
# >>> code
for a in b:
    a
    b
    c
else:
    pass
# <<<

# >>> error
NameError: name 'b' is not defined# <<<
# >>> error
NameError: name 'b' is not defined# <<<
# >>> error
NameError: name 'c' is not defined# <<<

# >>> case: 1
# >>> code
for a, (b, c), d in b:
    pass
# <<<

# >>> call
NameError: name 'b' is not defined# <<<

