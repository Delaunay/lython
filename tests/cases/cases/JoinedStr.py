# version=2
# > 
# >> code
a = f"str1 {b:4d}"# <<

# >> call
NameError: name 'b' is not defined# <<

# > 
# >> code
a = f"str1 {b:{s}d}"# <<

# >> error:: NameError: name 'b' is not defined
# >> error:: NameError: name 's' is not defined
# > 
# >> code
a = f"str1 {{ {b:{s}d} }}"# <<

# >> error:: NameError: name 'b' is not defined
# >> error:: NameError: name 's' is not defined
