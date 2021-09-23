import ast

def show(code):
    print(ast.dump(ast.parse(code), indent=4))

code = \
"""
a, b = c
"""

show(code)
