# version=2
# > 
# >> code
raise a from b# <<

# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# > 
# >> code
raise a# <<

# >> error:: NameError: name 'a' is not defined



# > 
# >> code
def fun():
    raise a from b
# <<

# >> call
fun()# <<

# > 
# >> code
def fun():
    raise a
# <<

# >> call
fun()# <<

