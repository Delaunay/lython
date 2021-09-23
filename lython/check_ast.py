import ast

def show(code):
    print(ast.dump(ast.parse(code), indent=4))

code = \
"""
for (i, *j) in range(10):
    pass
"""

show(code)
