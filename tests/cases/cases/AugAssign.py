# version=2
# > 
# >> code
a += b# <<

# >> SSA
a_#0:  = a + b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for Add: 'None' and 'None'
# > 
# >> code
a -= b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: TypeError: unsupported operand type(s) for Sub: 'None' and 'None'

# >> SSA
a_#0:  = a - b# <<
