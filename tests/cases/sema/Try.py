# >>> case: 0
# >>> code
try:
    pass
except Exception as b:
    pass
else:
    pass
finally:
    pass
# <<<

# >>> error
NameError: name 'Exception' is not defined# <<<
# >>> error
TypeError: expression `Exception` is not compatible with type `Type`# <<<

