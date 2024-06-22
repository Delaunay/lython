# version=2
# > 0
# >> code
return a# <<

# >> call
NameError: name 'a' is not defined# <<

# > 1
# >> code
return 1, 2# <<

# > 2
# >> code
return a + b# <<

# >> error
NameError: name 'a' is not defined# <<

# >> error
NameError: name 'b' is not defined# <<

# > 3
# >> code
return p.x + p.y# <<

# >> error
NameError: name 'p' is not defined# <<

# >> error
NameError: name 'p' is not defined# <<

