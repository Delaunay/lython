# version=2
# > 0
# >> code
assert a# <<

# >> error
NameError: name 'a' is not defined# <<

# > 1
# >> code
assert a, "b"# <<

# >> error
NameError: name 'a' is not defined# <<

