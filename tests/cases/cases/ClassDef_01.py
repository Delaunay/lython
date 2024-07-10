# version=2
# > 
# >> code
class CustomAnd:
    pass

a = CustomAnd()
a and True
# <<

# >> call
TypeError: unsupported operand type(s) for and: 'CustomAnd' and 'bool'# <<

# > 
# >> code
class CustomAnd:
    def __and__(self, b: bool) -> bool:
        return True

a = CustomAnd()
a and True
# <<
