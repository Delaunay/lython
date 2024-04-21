# >>> case: 0
# >>> code
return a# <<<

# >>> call
NameError: name 'a' is not defined# <<<

# >>> case: 1
# >>> code
return 1, 2# <<<

# >>> case: 2
# >>> code
return a + b# <<<

# >>> error
NameError: name 'a' is not defined# <<<
# >>> error
NameError: name 'b' is not defined# <<<

# >>> case: 3
# >>> code
return p.x + p.y# <<<

# >>> error
NameError: name 'p' is not defined# <<<
# >>> error
NameError: name 'p' is not defined# <<<

