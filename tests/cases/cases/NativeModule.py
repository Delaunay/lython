# version=2

# > Native Method 
# >> code
from nmodule import native_add
# <<
# >> call:: native_add(1, 2)
# >> result:: 3
# >> type:: int

# > Native Object Attribute
# >> code
from nmodule import Point
# <<
# >> call:: Point(1, 2).y
# >> result:: 2
# >> type:: int

# > Native Object Method
# >> code
from nmodule import Point
# <<
# >> call:: Point(1, 2).add(Point(1, 2)).y
# >> result:: 4
# >> type:: int
