


Design
=======


## Tokenize source code
  
code -*- my_source -*-
    
    def myfunction(x, y, z):
        (z + y) * x
        
token -*- my tokens -*-
    
    tokens = [
        0001: (tok: [-2, '�'], def: 'def')
        0002: (tok: [-4, '�'], ide: 'myfunction')
        0003: (tok: [40, '('], chr: '(')
        0004: (tok: [-4, '�'], ide: 'x')
        0005: (tok: [44, ','], chr: ',')
        0006: (tok: [-4, '�'], ide: 'y')
        0007: (tok: [44, ','], chr: ',')
        0008: (tok: [-4, '�'], ide: 'z')
        0009: (tok: [41, ')'], chr: ')')
        0010: (tok: [58, ':'], chr: ':')
        0011: (tok: [40, '('], chr: '(')
        0012: (tok: [-4, '�'], ide: 'x')
        0013: (tok: [43, '+'], chr: '+')
        0014: (tok: [-4, '�'], ide: 'y')
        0015: (tok: [41, ')'], chr: ')')
        0016: (tok: [42, '*'], chr: '*')
        0017: (tok: [-4, '�'], ide: 'z')
        0018: (tok: [59, ';'], chr: ';')
        0019: (tok: [-1, '�'], chr: '�')
    ] (size: 19)
    
## Parser
    
    Takes token and returns memory representation of the source as Expression
    
## Syntax Tree

    Implements memory representation of expression sources,
    implements method to generate IR code from the expression.

