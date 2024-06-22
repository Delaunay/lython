# version=2
# > 0
# >> code
+ a# <<

# >> error
NameError: name 'a' is not defined# <<

# >> error
TypeError: unsupported operand type(s) for UAdd: 'None' and 'None'# <<

# > 1
# >> code
- a# <<

# >> error
NameError: name 'a' is not defined# <<

# >> error
TypeError: unsupported operand type(s) for USub: 'None' and 'None'# <<

# > 2
# >> code
~ a# <<

# >> error
NameError: name 'a' is not defined# <<

# >> error
TypeError: unsupported operand type(s) for Invert: 'None' and 'None'# <<

# > 3
# >> code
! a# <<

# >> error
NameError: name 'a' is not defined# <<

# >> error
TypeError: unsupported operand type(s) for Not: 'None' and 'None'# <<

