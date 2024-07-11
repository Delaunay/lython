# version=2
# > 
# >> code
fun(a, b, c=d)# <<

# >> error:: NameError: name 'fun' is not defined
# >> error:: fun is not callable
# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: NameError: name 'd' is not defined

# > Function calls function
# >> code
def myfunction(a: f64, b: f64) -> f64:
    return a + b

def fun():
    return myfunction(1.0, 2.0)
# <<

# >> call
fun()# <<


# > call with nested expressions
# >> code
fun(1 + 2, sum([1, 2, 3]))# <<

# >> SSA
LY_0:  = 1 + 2
LY_1:  = [1, 2, 3]
LY_2:  = sum(LY_1)
fun(LY_0, LY_2)# <<

# > call is already SSA
# >> code
fun(1, 2, 3, a, b, c)# <<

# >> SSA
fun(1, 2, 3, a, b, c)# <<
