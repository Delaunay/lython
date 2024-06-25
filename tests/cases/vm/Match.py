# version=2
# > 
# >> code
def fun(a):
    match a:
        case [1, 3]:
            pass
        case b as c:
            return c
        case d | e:
            return d
        case ClassName(f, g, h=i):
            return f + g + i
        case j if k:
            return j
# <<

# >> call
fun()# <<

# > 
# >> code
def fun(lst):
    match lst:
        case []:
            pass
        case [head, *tail]:
            pass
# <<

# >> call
fun()# <<

# > 
# >> code
def fun(dct):
    match dct:
        case {}:
            pass
        case {1: value, **remainder}:
            pass
# <<

# >> call
fun()# <<

