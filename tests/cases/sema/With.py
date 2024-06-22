# version=2
# > 0
# >> code
with a as b, c as d:
    pass
# <<

# >> error
NameError: name 'a' is not defined# <<

# >> error
NameError: name 'c' is not defined# <<

# > 1
# >> code
with a as b, c as d:
    e = b + d
    e = b + d
    e = b + d
# <<

# >> error
NameError: name 'a' is not defined# <<

# >> error
NameError: name 'c' is not defined# <<

