from collections import defaultdict

from idl_parser import IDLParser, ParseFile, IDLLexer, IDLNode



called = False

def once(fun):

    def _(*args, **kwargs):
        global called
        if not called:
            # called = True
            return fun(*args, **kwargs)
    return _


@once
def dump(node: IDLNode):
    traverse_dump(node)
    print()

def traverse_dump(node: IDLNode, depth: int = 0):
    idt = "  " * depth
    nodestr = str(node)
    print(f"{idt} {nodestr:<{50 - len(idt)}} {node._properties}")
    for child in node._children:
        traverse_dump(child, depth + 1)


def get(attr: IDLNode, classname: str, default=None):
    for c in attr._children:
       if c.GetClass() == classname:
            return c
    return default


class Transform:
    def __init__(self, ast):
        self.nodes = defaultdict(list)
        
        for f in ast._children:
            assert f.GetClass() == "File"
            
            for child in f._children:
                
                method = getattr(self, "normalize_" + child.GetClass())
                
                self.nodes[child.GetClass()].append(method(child))
                

        for k, _ in self.nodes.items():
            print(f"{k:>15} {len(_):6d}    {_[0]}")
    
    def normalize_Interface(self, obj):
        flat = {
            "name": obj.GetName()
        }
        
        for pname, val in obj._properties.items():
            if pname not in ("LINENO", "POSITION", "WARNINGS", "ERRORS"):
                flat[pname] = val
        
        
        for child in obj._children:
            classname = child.GetClass()
            method = getattr(self, "normalize_" + classname)
                
            if classname not in flat:
                flat[classname] = []
            
            flat[classname].append(method(child))
                
        return flat
    
    def normalize_Constructor(self, obj):
        return obj
    
    def normalize_Attribute(self, obj):
        flat = {
            "name": obj.GetName(),
        }
        
        for child in obj._children:
            classname = child.GetClass()
            method = getattr(self, "normalize_" + classname)
            flat[classname] = method(child)
            
        return flat
    
    def normalize_Type(self, obj):
        flat = {}
        
        for child in obj._children:
            if "name" not in flat:
                flat["name"] = []
                
            if "class" not in flat:
                flat["class"] = []
                
            flat["name"].append(child.GetName())
            flat["class"].append(child.GetClass())
            
            for pname, pval in child._properties.items():
                if pname not in ("LINENO", "POSITION", "WARNINGS", "ERRORS", "FILENAME", "NAME"):
                    flat[pname] = pval
        
        return flat
    
    def normalize_Operation(self, obj):
        flat = {
            "name": obj.GetName(),
            "args": [],
            "return": "void"
        }
        
        arguments = get(obj, "Arguments", None)
        
        return_type = get(obj, "Type", None)
        
        if return_type is not None:
            flat["return"] = self.normalize_Type(return_type)
        
        if arguments is not None:
            flat["args"] = self.normalize_Arguments(arguments)
        
        return flat
    
    def normalize_Elipsis(self, obj):
        return obj is not None
    
    def normalize_Arguments(self, obj):
        args = []
        
        for child in obj._children:
            arg = {
                "name": child.GetName(),
                "type": self.normalize_Type(get(child, "Type")),
                "default": self.normalize_Default(get(child, "Default")),
                "variadic": self.normalize_Elipsis(get(child, "Argument"))
            }
            
            for pname, pval in child._properties.items():
                if pname not in ("LINENO", "POSITION", "WARNINGS", "ERRORS", "FILENAME", "NAME"):
                    arg[pname] = pval
        
            # print(arg)
            args.append(arg)
        
        return args
    
    def normalize_Default(self, obj):
        if obj is None:
            return None
    
        val = obj.GetProperty('VALUE')
        type = obj.GetProperty("TYPE")

        if type == "integer":
            pass
        elif type == "DOMString":
            val = f"\"{val}\""
        elif type == "dictionary":
            pass
        elif type == "NULL":
            # here we need to use `type` to figure out
            # what NULL translates to
            # if it is Any if needs to be some emscripten::NULL
            # if it is optional it needs to be {}
            val = "{}"
        elif type == "boolean":
            val = "true" if bool(val) else "false"
        elif type is None:
            pass
        elif type == "sequence":
            pass
        elif type == "float":
            pass
        else:
            print("UNHANDLED TYPE:", type)
    
        return {
            "value": val,
            "type": type
        }    
    
    def normalize_Inherit(self, obj):
        return obj.GetName()
    
    def normalize_Maplike(self, obj):
        key = self.normalize_Type(obj._children[0])
        val = self.normalize_Type(obj._children[1])
        
        return {
            "name": "dict",
            "key": key,
            "val": val
        }

    def normalize_Iterable(self, obj):
        return {
            "name": "iterable",
            "types": [self.normalize_Type(c) for c in obj._children]
        }
    
    def normalize_Const(self, obj):
        return {
            "name": obj.GetName(),
            "type": self.normalize_Type(obj._children[0]),
            "value": self.normalize_Default(obj._children[1]),
        }
    
    def normalize_Stringifier(self, obj):
        # dump(obj)
        return obj
    
    def normalize_Setlike(self, obj):
        # dump(obj)
        return obj
    
    def normalize_AsyncIterable(self, obj):
        # dump(obj)
        return obj
    
    def normalize_ExtAttributes(self, obj):
        # dump(obj)
        return obj
    
    def normalize_Includes(self, obj):
        # dump(obj)
        return obj
    
    def normalize_Dictionary(self, obj):
        # dump(obj)
        return obj
    
    def normalize_Enum(self, obj):
        # dump(obj)
        return obj
    
    def normalize_Typedef(self, obj):
        # dump(obj)
        return obj
    
    def normalize_Callback(self, obj):
        # dump(obj)
        return obj
    
    def normalize_Namespace(self, obj):
        dump(obj)
        return obj
        

class Generate:
    def __init__(self, nodes) -> None:
        for k, vv in nodes.items():
            for v in vv:        
                method = getattr(self, "generate_" + k)
                method(v)
        
    def generate_Interface(self, obj):
        pass


    def generate_Includes(self, obj):
        pass


    def generate_Namespace(self, obj):
        pass

    def generate_Dictionary(self, obj):
        pass

    def generate_Enum(self, obj):
        pass

    def generate_Typedef(self, obj):
        pass

    def generate_Callback(self, obj):
        pass


def main(argv):
    nodes = []
    parser = IDLParser(IDLLexer())
    errors = 0
    for filename in argv:
        filenode = ParseFile(parser, filename)
        if (filenode):
            errors += filenode.GetProperty('ERRORS')
            nodes.append(filenode)

    ast = IDLNode('AST', '__AST__', 0, 0, nodes)

    if errors:
        print('\nFound %d errors.\n' % errors)

    nodes = Transform(ast).nodes
    
    Generate(nodes)


if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv[1:]))
