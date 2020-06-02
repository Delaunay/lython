

def fibo_naive(n: Int) -> Int:
    if n == 0:
        return 0

    if n == 1:
        return 1

    return fibo_naive(n - 1) + fibo_naive(n - 2)


def fibo_loop(n: Int) -> Int:
    prev = 0
    current = 1

    for i in range(n):
        t = current + prev

        prev = current
        current = t
        
    return current


def fibo_tail(n: Int, current: Int = 1, prev: Int = 0) -> Int:
    if n == 0:
        return current

    return fibo_tail(n - 1, current + prev, current)



fibo_values: Dict[Int, Int] = {
    0: 0,
    1: 1,
}


def fibo_memoize(n: Int) -> Int:
    if not n in fibo_values:
        fibo_values[n] = fibo_memoize(n - 1) + fibo_memoize(n - 2)
        
    return fibo_values[n]    

