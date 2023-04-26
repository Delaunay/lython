import ast
from argparse import Namespace


class CPPGenerator(ast.NodeVisitor):
    def __init__(self):
        super(CPPGenerator, self).__init__()
        self.depth = 0

    def visit(self, node) -> str:
        self.depth += 1
        v = super().visit(node)
        self.depth -= 1
        return v

    def visit_arguments(self, node):
        return self.visit_children(node)

    def visit_arg(self, node):
        return self.visit_children(node)

    def visit_comprehension(self, node):
        
        return self.visit_children(node)

    def visit_excepthandler(self, node):
        
        return self.visit_children(node)

    def visit_keyword(self, node):
        return self.visit_children(node)

    def visit_alias(self, node):
        return self.visit_children(node)

    def visit_withitem(self, node):
        return self.visit_children(node)

    def visit_type_ignore(self, node):
        return self.visit_children(node)

    def visit_children(self, node):
        trace = ':'.join(['|' for _ in range(self.depth)])
        print(f'{trace} {self.depth} {node}')
        v = None
        for value in ast.iter_child_nodes(node):
            v = self.visit(value)
        return v

    def generic_visit(self, node):
        print(type(node), 'does not exist')
        return self.visit_children(node)

    # mod
    def visit_Module(self, node):
        
        return self.visit_children(node)

    def visit_Interactive(self, node):
        
        return self.visit_children(node)

    def visit_Expression(self, node):
        
        return self.visit_children(node)

    def visit_FunctionType(self, node):

        return self.visit_children(node)

    def get_expr(self, node):
        return Namespace(**dict(ast.iter_fields(node)))

    # stmt
    def visit_FunctionDef(self, node: ast.FunctionDef):
        fun_def: ast.FunctionDef = self.get_expr(node)
        all_args: ast.arguments = self.get_expr(fun_def.args)
        return_type = self.visit(fun_def.returns)

        args = []
        defaults = [None] * len(all_args.args)
        for i, val in enumerate(all_args.defaults):
            defaults[-(i + 1)] = val

        for arg, default in zip(all_args.args, defaults):
            arg_n = self.get_expr(arg)
            arg_type = self.visit(arg.annotation)
            arg_name = arg_n.arg

            if default is not None:
                default = self.visit(default)
                args.append(f'{arg_type} {arg_name} = {default}')
            else:
                args.append(f'{arg_type} {arg_name}')

        docstring = None
        expressions = []
        for i, expr in enumerate(fun_def.body):
            # Docstring
            if isinstance(expr, ast.Expr) and i == 0:
                data = self.get_expr(expr).value
                if isinstance(data, ast.Str):
                    docstring = self.get_expr(data).s
                    continue

            expressions.append(self.visit(expr) + ';')

        if docstring is not None:
            docstring = f'// {docstring}'
        else:
            docstring = ''

        args = ', '.join(args)
        exprs = '\n    '.join(expressions)
        return f"""
{docstring}
{return_type} {fun_def.name}({args}){{
    {exprs}
}}
"""

    def visit_AsyncFunctionDef(self, node):
        return self.visit_children(node)

    def visit_ClassDef(self, node):
        return self.visit_children(node)

    def visit_Return(self, node):
        ret: ast.Return = self.get_expr(node)
        return f'return {self.visit(ret.value)}'

    def visit_Delete(self, node):
        return self.visit_children(node)

    def visit_Assign(self, node):
        assign: ast.Assign = self.get_expr(node)
        targets = assign.targets
        value = assign.value

        targets_str = ','.join([self.visit(t) for t in targets])
        value_str = self.visit(value)

        return f'{targets_str} = {value_str}'

    def visit_AugAssign(self, node):
        return self.visit_children(node)

    def visit_AnnAssign(self, node):
        return self.visit_children(node)

    def visit_For(self, node):
        return self.visit_children(node)

    def visit_AsyncFor(self, node):
        return self.visit_children(node)

    def visit_While(self, node):
        return self.visit_children(node)

    def visit_If(self, node):
        return self.visit_children(node)

    def visit_With(self, node):
        return self.visit_children(node)

    def visit_AsyncWith(self, node):
        return self.visit_children(node)

    def visit_Raise(self, node):
        return self.visit_children(node)

    def visit_Try(self, node):
        return self.visit_children(node)

    def visit_Assert(self, node):
        return self.visit_children(node)

    def visit_Import(self, node):
        return self.visit_children(node)

    def visit_ImportFrom(self, node):
        return self.visit_children(node)

    def visit_Global(self, node):
        return self.visit_children(node)

    def visit_Nonlocal(self, node):
        return self.visit_children(node)

    def visit_Expr(self, node):
        return self.visit_children(node)

    def visit_Pass(self, node):
        return self.visit_children(node)

    def visit_Break(self, node):
        return self.visit_children(node)

    def visit_Continue(self, node):
        
        return self.visit_children(node)

    def visit_Bool(self, node):
        return self.visit_children(node)

    # Expression
    def visit_BoolOp(self, node):
        return self.visit_children(node)

    def visit_NamedExpr(self, node):
        return self.visit_children(node)

    def visit_BinOp(self, node):
        bin_op: ast.BinOp = self.get_expr(node)

        rhs = self.visit(bin_op.right)
        lhs = self.visit(bin_op.left)
        op = self.visit(bin_op.op)
        
        return f'{lhs} {op} {rhs}'

    def visit_UnaryOp(self, node):
        return self.visit_children(node)

    def visit_Lambda(self, node):
        return self.visit_children(node)

    def visit_IfExp(self, node):
        return self.visit_children(node)

    def visit_Dict(self, node):
        
        return self.visit_children(node)

    def visit_Set(self, node):
        return self.visit_children(node)

    def visit_ListComp(self, node):
        return self.visit_children(node)

    def visit_SetComp(self, node):
        return self.visit_children(node)

    def visit_DictComp(self, node):
        return self.visit_children(node)

    def visit_GeneratorExp(self, node):
        return self.visit_children(node)

    def visit_Await(self, node):
        return self.visit_children(node)

    def visit_Yield(self, node):
        return self.visit_children(node)

    def visit_YieldFrom(self, node):
        return self.visit_children(node)

    def visit_Num(self, node):
        num: ast.Num = self.get_expr(node)
        return num.n

    def visit_Str(self, node):
        
        return self.visit_children(node)

    def visit_Compare(self, node):
        
        return self.visit_children(node)

    def visit_Call(self, node):
        
        return self.visit_children(node)

    def visit_FormattedValue(self, node):
        
        return self.visit_children(node)

    def visit_JoinedStr(self, node):
        
        return self.visit_children(node)

    def visit_Constant(self, node):
        
        return self.visit_children(node)

    def visit_Attribute(self, node):
        
        return self.visit_children(node)

    def visit_Subscript(self, node):
        
        return self.visit_children(node)

    def visit_Starred(self, node):
        
        return self.visit_children(node)

    def visit_Name(self, node):
        name: ast.Name = self.get_expr(node)
        return name.id

    def visit_List(self, node):
        
        return self.visit_children(node)

    def visit_Tuple(self, node):
        
        return self.visit_children(node)

    def visit_Load(self, node):
        
        return self.visit_children(node)

    def visit_Store(self, node):
        
        return self.visit_children(node)

    def visit_Del(self, node):
        
        return self.visit_children(node)

    def visit_AugLoad(self, node):
        
        return self.visit_children(node)

    def visit_AugStore(self, node):
        
        return self.visit_children(node)

    def visit_Param(self, node):
        
        return self.visit_children(node)

    def visit_Slice(self, node):
        
        return self.visit_children(node)

    def visit_ExtSlice(self, node):
        
        return self.visit_children(node)

    def visit_Index(self, node):
        
        return self.visit_children(node)

    def visit_And(self, node):
        
        return self.visit_children(node)

    def visit_Or(self, node):
        
        return self.visit_children(node)

    def visit_Add(self, node):
        # add: ast.Add = self.get_expr(node)
        return '+'

    def visit_Sub(self, node):
        
        return self.visit_children(node)

    def visit_Mult(self, node):
        
        return self.visit_children(node)

    def visit_MatMult(self, node):
        
        return self.visit_children(node)

    def visit_Div(self, node):
        
        return self.visit_children(node)

    def visit_Mod(self, node):
        
        return self.visit_children(node)

    def visit_Pow(self, node):
        
        return self.visit_children(node)

    def visit_LShift(self, node):
        
        return self.visit_children(node)

    def visit_RShift(self, node):
        
        return self.visit_children(node)

    def visit_BitOr(self, node):
        
        return self.visit_children(node)

    def visit_BitXor(self, node):
        
        return self.visit_children(node)

    def visit_BitAnd(self, node):
        
        return self.visit_children(node)

    def visit_FloorDiv(self, node):
        
        return self.visit_children(node)

    def visit_Invert(self, node):
        
        return self.visit_children(node)

    def visit_Not(self, node):
        
        return self.visit_children(node)

    def visit_UAdd(self, node):
        
        return self.visit_children(node)

    def visit_USub(self, node):
        
        return self.visit_children(node)

    def visit_Eq(self, node):
        
        return self.visit_children(node)

    def visit_NotEq(self, node):
        
        return self.visit_children(node)

    def visit_Lt(self, node):
        
        return self.visit_children(node)

    def visit_LtE(self, node):
        
        return self.visit_children(node)

    def visit_Gt(self, node):
        
        return self.visit_children(node)

    def visit_GtE(self, node):
        
        return self.visit_children(node)

    def visit_Is(self, node):
        
        return self.visit_children(node)

    def visit_IsNot(self, node):
        
        return self.visit_children(node)

    def visit_In(self, node):
        
        return self.visit_children(node)

    def visit_NotIn(self, node):
        
        return self.visit_children(node)


import sys
sys.stderr = sys.stdout

parsed = ast.parse('''

@dummy
def add(a: float, b: float, c: float = 1) -> float:
    """Add Numbers"""
    d = a + b
    return d + c
''')

visitor = CPPGenerator()
print(visitor.visit(parsed))

# print(parsed)
#
# for node in ast.walk(parsed):
#     
#     for name, value in ast.iter_fields(node):
#         print(f' - {name} {value}')
#

