# >>> case: 0
# >>> code
a, b, c# <<<

# >>> error
NameError: name 'a' is not defined# <<<
# >>> error
NameError: name 'b' is not defined# <<<
# >>> error
NameError: name 'c' is not defined# <<<

# >>> case: 1
# >>> code
a, (b, c), d# <<<

# >>> error
NameError: name 'a' is not defined# <<<
# >>> error
NameError: name 'b' is not defined# <<<
# >>> error
NameError: name 'c' is not defined# <<<
# >>> error
NameError: name 'd' is not defined# <<<

# >>> case: 2
# >>> code
a, b, c = d, e, f# <<<

# >>> error
NameError: name 'd' is not defined# <<<
# >>> error
NameError: name 'e' is not defined# <<<
# >>> error
NameError: name 'f' is not defined# <<<

