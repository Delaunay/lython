import ast


def show(code):
    print(ast.dump(ast.parse(code), indent=4))


"""
def a(b, c=d, *e, f=g, **h) -> i:
    pass
"""

code = """
a + b
"""
code = """
f"{a:2d} a {b:6.2f}"
"""

code = """
a and b and c and d
"""


code = """

def fun(a, b, c=1, *args, d=e, f=g, **kwargs):
    pass

call(a, b, c=1, *args, d=e, f=c, **kwargs)
"""

show(code)
