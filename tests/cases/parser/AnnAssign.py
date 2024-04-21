# >>> case: 0
# >>> code
a: bool = True# <<<

# >>> case: 1
# >>> code
a: int = 1# <<<

# >>> error
NameError: name 'int' is not defined# <<<
# >>> error
TypeError: expression `int` is not compatible with type `Type`# <<<
# >>> error
TypeError: expression `a` of type `int` is not compatible with expression `1` of type `i32`# <<<

# >>> case: 2
# >>> code
a: isnt = 1# <<<

# >>> error
NameError: name 'isnt' is not defined# <<<
# >>> error
TypeError: expression `isnt` is not compatible with type `Type`# <<<
# >>> error
TypeError: expression `a` of type `isnt` is not compatible with expression `1` of type `i32`# <<<

# >>> case: 3
# >>> code
a: f32 = 2.0# <<<

# >>> error
TypeError: expression `a` of type `f32` is not compatible with expression `2.0` of type `f64`# <<<

