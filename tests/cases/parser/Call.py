# >>> case: 0
# >>> code
fun(a, b, c=d)# <<<

# >>> error
NameError: name 'fun' is not defined# <<<
# >>> error
fun is not callable# <<<
# >>> error
NameError: name 'a' is not defined# <<<
# >>> error
NameError: name 'b' is not defined# <<<
# >>> error
NameError: name 'd' is not defined# <<<

