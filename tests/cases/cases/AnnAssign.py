# version=2
# > 
# >> code
a: bool = True# <<

# > 
# >> code
a: int = 1# <<

# >> error:: NameError: name 'int' is not defined
# >> error:: TypeError: expression `int` is not compatible with type `Type`
# >> error:: TypeError: expression `a` of type `int` is not compatible with expression `1` of type `i32`
# > 
# >> code
a: isnt = 1# <<

# >> error:: NameError: name 'isnt' is not defined
# >> error:: TypeError: expression `isnt` is not compatible with type `Type`
# >> error:: TypeError: expression `a` of type `isnt` is not compatible with expression `1` of type `i32`
# > 
# >> code
a: f32 = 2.0# <<

# >> error:: TypeError: expression `a` of type `f32` is not compatible with expression `2.0` of type `f64`

# > 
# >> code
def fun() -> bool:
    a: bool = True
    return a
# <<

# >> call:: fun()
# >> result:: True
# >> type:: bool

# > 
# >> code
def fun() -> i32:
    a: i32 = 1
    return a
# <<

# >> call:: fun()
# >> result:: 1
# >> type:: i32

# > 
# >> code
def fun() -> f64:
    a: f64 = 2.0
    return a
# <<

# >> call:: fun()
# >> result:: 2.0
# >> type:: f64


# > case: VM_AnnAssign
# >> code
def fun(a: i32) -> i32:
    b: i32 = 3
    b: i32 = a * b
    return b
# <<

# >> call:: fun(2)
# >> result:: 6
# >> type:: i32
