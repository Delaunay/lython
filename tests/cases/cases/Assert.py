# version=2
# > 
# >> code
assert a# <<

# >> error:: NameError: name 'a' is not defined
# > 
# >> code
assert a, "b"# <<

# >> error:: NameError: name 'a' is not defined
