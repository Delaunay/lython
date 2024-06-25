# version=2
# > 
# >> code
a = c if True else d# <<

# >> error:: NameError: name 'c' is not defined
# >> error:: NameError: name 'd' is not defined
