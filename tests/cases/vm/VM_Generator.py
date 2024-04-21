# >>> case: VM_Generator
# >>> code
def range(n: i32) -> i32:
    c: i32 = 0
    while c < n:
        yield c
        c += 1

def fun() -> i32:
    acc: i32 = 0
    for i in range(10):
        acc += i
    return acc
# <<<


# >>> call
fun()# <<<


# >>> expected
45# <<<


