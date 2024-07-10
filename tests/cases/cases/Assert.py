# version=2
# > 
# >> code
assert a# <<

# >> error:: NameError: name 'a' is not defined
# > 
# >> code
assert a, "b"# <<

# >> error:: NameError: name 'a' is not defined


# > case: VM_assert_True
# >> code
def fun(a: i32) -> i32:
    assert True, "all good"
    return 1
# <<


# >> call:: fun(0)
# >> result:: 1

# > case: VM_assert_False
# >> code
def fun(a: i32) -> i32:
    assert False, "Very bad"
    return 1
# <<

# >> call:: fun(0)
# >> result
Traceback (most recent call last):
  File "<input>", line -2, in <module>
    fun(0)
  File "<input>", line 2, in fun
    assert False, "Very bad"
AssertionError: Very bad
# <<


# > 
# >> code
def fun(a: int):
    assert a
# <<

# >> call
fun()# <<

# > 
# >> code
def fun(a: int):
    assert a, "b"# <<

# >> call
fun()# <<
