import ast


def show(code):
    print(ast.dump(ast.parse(code), indent=4))



"""
def a(b, c=d, *e, f=g, **h) -> i:
    pass
"""

code = \
"""
a + b
"""
code = \
"""
f"{a:2d} a {b:6.2f}"
"""

show(code)
