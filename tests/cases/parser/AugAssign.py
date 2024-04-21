# >>> case: 0
# >>> code
a += b# <<<

# >>> error
NameError: name 'a' is not defined# <<<
# >>> error
NameError: name 'b' is not defined# <<<
# >>> error
TypeError: unsupported operand type(s) for Add: 'None' and 'None'# <<<

# >>> case: 1
# >>> code
a -= b# <<<

# >>> error
NameError: name 'a' is not defined# <<<
# >>> error
NameError: name 'b' is not defined# <<<
# >>> error
TypeError: unsupported operand type(s) for Sub: 'None' and 'None'# <<<

