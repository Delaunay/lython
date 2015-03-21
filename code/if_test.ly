extern cos(x)
extern sin(x)

def baz(x):
    if x < 0:
        baz(x) 
    else:
        baz(x);
