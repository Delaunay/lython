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



code = """
a = f"{100:1d} normal  {1.123:10.2f} normal {'str':>25}"
"""


code = """
d = 4
n = f'{1:0d}'
"""

code = """
a = f"{d =}"

"""
show(code)


n = 1.123
a = f'{n:c=+#0.2f}'
print()
print(a)
