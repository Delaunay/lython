# version=2
# > 0
# >> code
raise a from b# <<

# >> error
NameError: name 'a' is not defined# <<

# >> error
NameError: name 'b' is not defined# <<

# > 1
# >> code
raise a# <<

# >> error
NameError: name 'a' is not defined# <<

