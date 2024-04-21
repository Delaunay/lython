# >>> case: 0
# >>> code
a = b# <<<

# >>> call
NameError: name 'b' is not defined# <<<

# >>> case: 1
# >>> code
a, b = c# <<<

# >>> call
NameError: name 'c' is not defined# <<<

# >>> case: 2
# >>> code
a = 1# <<<

# >>> expected
i32# <<<


# >>> case: 3
# >>> code
a = 1.0# <<<

# >>> expected
f64# <<<


# >>> case: 4
# >>> code
a = "str"# <<<

# >>> expected
str# <<<


# >>> case: 5
# >>> code
a = [1, 2]# <<<

# >>> expected
Array[i32]# <<<


# >>> case: 6
# >>> code
a = [1.0, 2.0]# <<<

# >>> expected
Array[f64]# <<<


# >>> case: 7
# >>> code
a = ["1", "2"]# <<<

# >>> expected
Array[str]# <<<


# >>> case: 8
# >>> code
a = {1, 2}# <<<

# >>> expected
Set[i32]# <<<


# >>> case: 9
# >>> code
a = {1.0, 2.0}# <<<

# >>> expected
Set[f64]# <<<


# >>> case: 10
# >>> code
a = {"1", "2"}# <<<

# >>> expected
Set[str]# <<<


# >>> case: 11
# >>> code
a = {1: 1, 2: 2}# <<<

# >>> expected
Dict[i32, i32]# <<<


# >>> case: 12
# >>> code
a = {1: 1.0, 2: 2.0}# <<<

# >>> expected
Dict[i32, f64]# <<<


# >>> case: 13
# >>> code
a = {"1": 1, "2": 2}# <<<

# >>> expected
Dict[str, i32]# <<<


# >>> case: 14
# >>> code
a = 1, 2.0, "str"# <<<

# >>> expected
Tuple[i32, f64, str]# <<<


