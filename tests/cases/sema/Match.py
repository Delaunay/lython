# >>> case: 0
# >>> code
match a:
    case [1, 3]:
        return "case 1"
    case 1 | 2:
        return "case 2"
    case ClassName(f, g, h=i):
        return 'case 3'
    case j if j > k:
        return "case 4"
    case b as c if c == k:
        return "case 5"
    case _:
        return "case 6"
# <<<

# >>> error
NameError: name 'a' is not defined# <<<
# >>> error
NameError: name 'ClassName' is not defined# <<<
# >>> error
NameError: name 'k' is not defined# <<<
# >>> error
TypeError: unsupported operand type(s) for Gt: 'None' and 'None'# <<<
# >>> error
NameError: name 'k' is not defined# <<<
# >>> error
TypeError: unsupported operand type(s) for Eq: 'None' and 'None'# <<<

# >>> case: 1
# >>> code
match lst:
    case []:
        pass
    case [head, *tail]:
        pass
# <<<

# >>> error
NameError: name 'lst' is not defined# <<<

# >>> case: 2
# >>> code
match dct:
    case {}:
        pass
    case {1: value, **remainder}:
        pass
# <<<

# >>> error
NameError: name 'dct' is not defined# <<<

