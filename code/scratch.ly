def myfunction(a: f64, b: f64) -> f64:
    return a + b


def fun():
    return myfunction(1.0, 2.0)


def cond(a: bool):
    if a:
        return 0.0
    else:
        return 1.0

    return 2.0


class Point:
    x: i32
    y: f64
    z: i8
