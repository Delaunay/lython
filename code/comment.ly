# c0
for i in range(0, 10): # c1
    s += 1             # c2
    a += b             # c3
else: #c4
    pass # c5
# c6


# c0
while True: # c1
    s += 1             # c2
    a += b             # c3
else: #c4
    pass # c5
# c6

# c0
with a as b: # c1
    pass # c2
# c3

# c0
try:# c1
    pass# c2
except Exception as b:# c3
    pass# c4
else:# c5
    pass# c6
finally:# c7
    pass# c8
# c9

# c0
if True: # c1
    pass # c2
elif True: #c3
    pass # c4
elif False: # c5
    pass # c6
else: # c7
    pass # c8
# c9

# c0
match a: # c1
    case a:# c2
        return b # c3
# c4

import abc # c1
import aa as b, c as d, e.f as g # c1
from aa.b import c as d, e.f as g # c1

raise # c1
raise a # c1
raise a from b # c1
return # c1
return a # c1


def a(b, c=d, *e, f=g, **h) -> bool: #c1
    pass  #c2


class Name: # c1
    x: i32 # c2


# yield a # c1
# yield b from c #c1