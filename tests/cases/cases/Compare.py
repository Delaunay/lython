# version=2
# > 
# >> code
a < b > c != d# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for Lt: 'None' and 'None'
# >> error:: NameError: name 'c' is not defined
# >> error:: TypeError: unsupported operand type(s) for Gt: 'None' and 'None'
# >> error:: NameError: name 'd' is not defined
# >> error:: TypeError: unsupported operand type(s) for NotEq: 'None' and 'None'
# > 
# >> code
a not in b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for NotIn: 'None' and 'None'
# > 
# >> code
a in b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for In: 'None' and 'None'
# > 
# >> code
a is b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for Is: 'None' and 'None'
# > 
# >> code
a is not b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for IsNot: 'None' and 'None'
