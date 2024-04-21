# >>> case: VM_assert_True
# >>> code
def fun(a: i32) -> i32:
    assert True, "all good"
    return 1
# <<<


# >>> call
fun(0)# <<<


# >>> expected
1# <<<


# >>> case: VM_assert_False
# >>> code
def fun(a: i32) -> i32:
    assert False, "Very bad"
    return 1
# <<<


# >>> call
fun(0)# <<<


# >>> expected
Traceback (most recent call last):
  File "<input>", line -2, in <module>
    fun(0)
  File "<input>", line 2, in fun
    assert False, "Very bad"
AssertionError: Very bad
# <<<


