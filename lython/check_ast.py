import ast

def show(code):
    print(ast.dump(ast.parse(code), indent=4))



"""
def a(b, c=d, *e, f=g, **h) -> i:
    pass
"""

code = \
"""
def a(b, c, *e, k, f=g, **h) -> i:
    pass
"""

show(code)
