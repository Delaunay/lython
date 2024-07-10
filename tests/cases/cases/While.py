# version=2
# > 
# >> code
while a:
    pass
else:
    pass
# <<

# >> call
NameError: name 'a' is not defined# <<


# > case: VM_While
# >> code
def fun(a: i32) -> i32:
    b = 0
    while a > 0:
        a -= 1
        b += 2
    return b
# <<


# >> call
fun(2)# <<


# >> expected
4# <<


# > case: VM_While_continue
# >> code
def fun(a: i32) -> i32:
    b = 0
    while a > 0:
        a -= 1
        b += 1
        continue
        b += 1
    return b
# <<


# >> call
fun(10)# <<


# >> expected
10# <<


# > case: VM_While_break
# >> code
def fun(a: i32) -> i32:
    b = 0
    while a > 0:
        a -= 1
        b += 1
        break
        b += 1
    return b
# <<


# >> call
fun(10)# <<


# >> expected
1# <<


# > 
# >> code
def fun(limit: i32):
    a = 0
    while a < limit:
        a += 1
    else:
        pass
# <<

# >> call
fun(10)# <<

