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

# >> expected
Set[str]# <<

# > 
# >> code
a = {1: 1, 2: 2}# <<

# >> expected
Dict[i32, i32]# <<

# > 
# >> code
a = {1: 1.0, 2: 2.0}# <<

# >> expected
Dict[i32, f64]# <<

# > 
# >> code
a = {"1": 1, "2": 2}# <<

# >> expected
Dict[str, i32]# <<

# > 
# >> code
a = 1, 2.0, "str"# <<

# >> expected
Tuple[i32, f64, str]# <<

