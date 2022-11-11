def fun():
    return x

def fun(a: i32) -> i32:
    return a
x = fun(1)

def fun(a: i32) -> i32:
    return a
x: i32 = fun(1)

def fun(a: i32) -> i32:
    return a
x = fun(1.0)

def fun(a: i32) -> i32:
    return a
x: f32 = fun(1)

def fun(a: i32, b: f64) -> i32:
    return a
x: i32 = fun(b=1.0, a=1)

def fun(a: i32 = 1, b: f64 = 1.1) -> i32:
    return a
x: i32 = fun()

def fun(a: i32, b: f64 = 1.1) -> i32:
    return a
x: i32 = fun()

class Name:
    pass

a = Name()
a.x
a.x = 2

class Custom:
    def __init__(self, a: i32):
        self.a = a

a = Custom(1)

class Name:
    def __init__(self, x: i32):
        self.x = x

a = Name(2)
a.x
a.x = 4

def fun():
    return 1, 2, 3
a = fun()

def fun():
    return 1, 2, 3
a, b, c = fun()

def fun():
    return 1, 2, 3
a, *b = fun()

class Name:
    x: i32 = 1

a = Name()
a.x = 2

class CustomAnd:
    pass

a = CustomAnd()
a and True

class CustomAnd:
    def __and__(self, b: bool) -> bool:
        return True

a = CustomAnd()
a and True

a and b
a or b
a or b or c
a = b := c
a + b
a - b
a * b
a << b
a ^ b
+ a
- a
~ a
! a
lambda a: b
a = c if True else d
{a: b, c: d}
{a, b}
[a for a in b if a > c]
(a for a in b if a > c)
{a for a in b if a > c}
{a: c for a in b if a > c}
await a
yield a
yield 1, 2
yield
yield from a
a < b > c != d
a not in b
a in b
a is b
a is not b
fun(a, b, c=d)
1
2.1
"str"
None
True
False
a.b
a[b]
*a
a
[a, b, c]
a, b, c
a, (b, c), d
a, b, c = d, e, f
a[b:c:d]
@j
def a(b, c=d, *e, f=g, **h) -> bool:
    """docstring"""
    return True
@j(l, m, c=n)
@k
def a(b: bool, d: bool = True):
    pass
@e(g, h, i=j)
@f
class a(b, c=d):
    """docstring"""
    pass
class Name:
    x: i32 = 0
    y: i32 = 1
    z = 1.2

    def __init__(self):
        self.x = 2

return a
return 1, 2
return a + b
return p.x + p.y
del a, b
a = b
a, b = c
a = 1
a = 1.0
a = "str"
a = [1, 2]
a = [1.0, 2.0]
a = ["1", "2"]
a = {1, 2}
a = {1.0, 2.0}
a = {"1", "2"}
a = {1: 1, 2: 2}
a = {1: 1.0, 2: 2.0}
a = {"1": 1, "2": 2}
a = 1, 2.0, "str"
a += b
a -= b
a: bool = True
a: int = 1
a: isnt = 1
a: f32 = 2.0
for a in b:
    a
    b
    c
else:
    pass

for a, (b, c), d in b:
    pass

while a:
    pass
else:
    pass

if a:
    pass
elif b:
    pass
else:
    pass

with a as b, c as d:
    pass

with a as b, c as d:
    e = b + d
    e = b + d
    e = b + d

raise a from b
raise a
try:
    pass
except Exception as b:
    pass
else:
    pass
finally:
    pass

assert a
assert a, "b"
import aa as b, c as d, e.f as g
import import_test as imp_test
from aa.b import c as d, e.f as g
from import_test import cls as Klass, fun as Fun, ann as Ann, var as Var
global a
nonlocal a
pass
break
continue
match a:
    case [1, 3]:
        pass
    case b as c:
        return c
    case d | e:
        return d
    case ClassName(f, g, h=i):
        return f + g + i
    case j if k:
        return j

match lst:
    case []:
        pass
    case [head, *tail]:
        pass

match dct:
    case {}:
        pass
    case {1: value, **remainder}:
        pass

a = 2; b = c; d = e
