# version=2
# > 0
# >> code
a < b > c != d# <<

# >> error
NameError: name 'a' is not defined# <<

# >> error
NameError: name 'b' is not defined# <<

# >> error
TypeError: unsupported operand type(s) for Lt: 'None' and 'None'# <<

# >> error
NameError: name 'c' is not defined# <<

# >> error
TypeError: unsupported operand type(s) for Gt: 'None' and 'None'# <<

# >> error
NameError: name 'd' is not defined# <<

# >> error
TypeError: unsupported operand type(s) for NotEq: 'None' and 'None'# <<

# > 1
# >> code
a not in b# <<

# >> error
NameError: name 'a' is not defined# <<

# >> error
NameError: name 'b' is not defined# <<

# >> error
TypeError: unsupported operand type(s) for NotIn: 'None' and 'None'# <<

# > 2
# >> code
a in b# <<

# >> error
NameError: name 'a' is not defined# <<

# >> error
NameError: name 'b' is not defined# <<

# >> error
TypeError: unsupported operand type(s) for In: 'None' and 'None'# <<

# > 3
# >> code
a is b# <<

# >> error
NameError: name 'a' is not defined# <<

# >> error
NameError: name 'b' is not defined# <<

# >> error
TypeError: unsupported operand type(s) for Is: 'None' and 'None'# <<

# > 4
# >> code
a is not b# <<

# >> error
NameError: name 'a' is not defined# <<

# >> error
NameError: name 'b' is not defined# <<

# >> error
TypeError: unsupported operand type(s) for IsNot: 'None' and 'None'# <<

