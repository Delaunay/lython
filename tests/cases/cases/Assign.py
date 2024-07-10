# version=2
# > 
# >> code
a = b# <<

# >> call
NameError: name 'b' is not defined# <<

# > 
# >> code
a, b = c# <<

# >> call
NameError: name 'c' is not defined# <<

# > 
# >> code
a = 1# <<

# >> expected
i32# <<

# > 
# >> code
a = 1.0# <<

# >> expected
f64# <<

# > 
# >> code
a = "str"# <<

# >> expected
str# <<

# > 
# >> code
a = [1, 2]# <<

# >> expected
Array[i32]# <<

# > 
# >> code
a = [1.0, 2.0]# <<

# >> expected
Array[f64]# <<

# > 
# >> code
a = ["1", "2"]# <<

# >> expected
Array[str]# <<

# > 
# >> code
a = {1, 2}# <<

# >> expected
Set[i32]# <<

# > 
# >> code
a = {1.0, 2.0}# <<

# >> expected
Set[f64]# <<

# > 
# >> code
a = {"1", "2"}# <<
# >> type:: Set[str]

# > Dict[i32, i32]
# >> code
a = {1: 1, 2: 2}# <<
# >> type:: Dict[i32, i32]

# > Dict[i32, f64]
# >> code
a = {1: 1.0, 2: 2.0}# <<
# >> type:: Dict[i32, f64]

# > Dict
# >> code
a = {"1": 1, "2": 2}# <<
# >> type:: Dict[str, i32]

# > Tuple
# >> code
a = 1, 2.0, "str"# <<
# >> type:: Tuple[i32, f64, str]

# > case: VM_assign
# >> code
def fun(a: i32) -> i32:
    b = 3
    b = a * b
    return b
# <<


# >> call:: fun(2)
# >> result:: 6


# > 
# >> code
def fun(b: i32) -> i32:
    a = b
    return a
# <<

# >> call
fun(1)# <<

# > 
# >> code
def fun(c: Tuple[i32, i32]) -> i32:
    a, b = c
    return a
# <<

# >> call
fun((1, 2))# <<

# > 
# >> code
def fun() -> i32:
    a = 1
    return a
# <<

# >> call
fun()# <<

# >> expected
i32# <<


# > 
# >> code
def fun():
    a = 1.0
    return a
# <<

# >> call
fun()# <<

# >> expected
f64# <<


