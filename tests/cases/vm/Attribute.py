# version=2
# > 
# >> code
class Point:
    x: i32
    y: i32

def fun(a: Point) -> i32:
    return a.x
# <<

# >> call
fun(Point())# <<

