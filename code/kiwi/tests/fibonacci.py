

def fibo_naive(n: i32) -> i32:
    if n == 0:
        return 0

    if n == 1:
        return 1

    return fibo_naive(n - 1) + fibo_naive(n - 2)


def fibo_loop(n: i32) -> i32:
    prev = 0
    current = 1

    for i in range(n):
        t = current + prev

        prev = current
        current = t
        
    return current


def fibo_tail(n: i32, current: i32 = 1, prev: i32 = 0) -> i32:
    if n == 0:
        return current

    return fibo_tail(n - 1, current + prev, current)



fibo_values: Dict[i32, i32] = {
    0: 0,
    1: 1,
}


def fibo_memoize(n: i32) -> i32:
    if not n in fibo_values:
        fibo_values[n] = fibo_memoize(n - 1) + fibo_memoize(n - 2)
        
    return fibo_values[n]    



fibo_naive(10)
