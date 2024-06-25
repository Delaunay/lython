# version=2
# > 
# >> code
for a in b:
    a
    b
    c
else:
    pass
# <<

# >> error:: NameError: name 'b' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: NameError: name 'c' is not defined
# > 
# >> code
for a, (b, c), d in b:
    pass
# <<

# >> call
NameError: name 'b' is not defined# <<

