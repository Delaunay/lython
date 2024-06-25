# version=2
# > 
# >> code
+ a# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: TypeError: unsupported operand type(s) for UAdd: 'None' and 'None'
# > 
# >> code
- a# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: TypeError: unsupported operand type(s) for USub: 'None' and 'None'
# > 
# >> code
~ a# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: TypeError: unsupported operand type(s) for Invert: 'None' and 'None'
# > 
# >> code
! a# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: TypeError: unsupported operand type(s) for Not: 'None' and 'None'
