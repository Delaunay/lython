# > case: VM_exception_stop_recursion
# >> code
def fun(a: i32) -> i32:
    if a == 0:
        assert False, "Very bad"
    return fun(a - 1)
# <<

# >> call
fun(2)# <<

# >> result
Traceback (most recent call last):
  File "<input>", line -2, in <module>
    fun(2)
  File "<input>", line 4, in fun
    return fun(a - 1)
  File "<input>", line 4, in fun
    return fun(a - 1)
  File "<input>", line 3, in fun
    assert False, "Very bad"
AssertionError: Very bad
# <<


# > case: VM_exception_stop_loop
# >> code
def fun(a: i32) -> i32:
    while True:
        assert False, "Very bad"
    return 1
# <<

# >> call
fun(2)# <<

# >> result
Traceback (most recent call last):
  File "<input>", line -2, in <module>
    fun(2)
  File "<input>", line 3, in fun
    assert False, "Very bad"
AssertionError: Very bad
# <<


