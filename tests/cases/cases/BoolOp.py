# version=2
# > 
# >> code
a and b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for and: 'None' and 'None'
# > 
# >> code
a or b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for or: 'None' and 'None'
# > 
# >> code
a or b or c# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for or: 'None' and 'None'
# >> error:: NameError: name 'c' is not defined
# >> error:: TypeError: unsupported operand type(s) for or: 'None' and 'None'
