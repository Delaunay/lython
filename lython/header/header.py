import os
from collections import defaultdict
from pathlib import Path

import clang.cindex



class Preprocessor:
    def __init__(self, folder, file=None) -> None:
        self.index = clang.cindex.Index.create() 
        self.tu = []
        self.folder = folder
        self.ignored = defaultdict(int)
        self.annotations = []
        self.structs = dict()
        self.custom_annotation = []
        self.file = file
        self.macros = {
            "KWMETA"
        }
        
        self.history = []
        self.parse_files(folder, file)
        for tu in self.tu:
            # print(tu)
            # self.recurse(tu.cursor)
            self.find_class_struct_members(tu.cursor, 0)

        self.generate_data()

    def recurse(self, node, depth=0):
        # self.history.append(node)
        
        if node.kind == clang.cindex.CursorKind.ANNOTATE_ATTR:
            print(node.spelling, node.kind)

        for child in node.get_children():
            print(" " * depth, child.spelling, child.kind)
            self.recurse(child, depth + 1)

    def parse_files(self, folder, file=None):
        files = [file] if file else list(Path(folder).glob('**/*.h'))
        for f in files:
            args = [
                '-x','c++', 
                '-std=c++20', 
                "-nostdinc", 
                "-I", str(folder),
                "-DKMETA_PROCESSING=1"
            ] 
            tu = self.index.parse(
                str(f), 
                args=args, 
                options=
                clang.cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD   | 
                clang.cindex.TranslationUnit.PARSE_INCOMPLETE                   |
                clang.cindex.TranslationUnit.PARSE_SKIP_FUNCTION_BODIES
            )
            self.tu.append(tu)
            self.check_for_errors(tu)


    def reflect_type(self, struct) -> bool:
        return struct["members"] and struct["ann"]
    
    def reflect_property(self, attr) -> bool:
        return attr.get("ann")
    
    def reflect_method(self, method) -> bool:
        return method.get("ann")
    
    def check_for_errors(self, tu):
        # Check for parsing errors
        for diag in tu.diagnostics:
            print(f"Diagnostic: {diag.severity} {diag.spelling}")
            if diag.severity == clang.cindex.Diagnostic.Error:
                print("Error:", diag.spelling)
            elif diag.severity == clang.cindex.Diagnostic.Warning:
                print("Warning:", diag.spelling)
            elif diag.severity == clang.cindex.Diagnostic.Note:
                print("Note:", diag.spelling)
            elif diag.severity == clang.cindex.Diagnostic.Fatal:
                print("Fatal error:", diag.spelling)
                
    def generate_data(self):

        with open(os.path.join(self.folder, "ast/meta.generated.cpp"), "w") as fp:
            print("#include \"dtypes.h\"", file=fp)
            print("#include \"ast/nodes.h\"", file=fp)
            print("#include \"utilities/names.h\"", file=fp)
            print("", file=fp)

            print("namespace lython {", file=fp)
            for name, struct in self.structs.items():
                if self.reflect_type(struct):
                    self.struct_begin(struct, fp)
                    
                    for attr in struct["members"]:
                        if self.reflect_property(attr):
                            self.generate_proprety(struct, attr, fp)
                    
                    for attr in struct["methods"]:
                        if self.reflect_method(attr):
                            self.generate_method(struct, attr, fp)

                    self.struct_end(struct, fp)


            print("}", file=fp)


    def struct_begin(self, struct, fp):
        typename = struct['name']

        print(f"template <>", file=fp)
        print(f"struct meta::ReflectionTrait<{typename}> {{", file=fp)
        print(f"    static int register_members() {{", file=fp)
    
    def struct_end(self, struct, fp):
        print(f"        return 1;", file=fp)
        print(f"    }}", file=fp)
        print(f"}};", file=fp)

    def generate_proprety(self, struct, attr, fp):
        attrname = attr["name"]
        typename = struct['name']
        print(f"        meta::register_member<&{typename}::{attrname}>(\"{attrname}\");", file=fp)

    def generate_method(self, struct, attr, fp):
        attrname = attr["name"]
        typename = struct['name']
        print(f"        meta::register_member<&{typename}::{attrname}>(\"{attrname}\");", file=fp)

    def parse_annotation(self, ann):
        return ann

    def add_field(self, members, node, gather_members, field_ann):
        ann = None
        for attr in node.get_children():
            if attr.kind == clang.cindex.CursorKind.ANNOTATE_ATTR:
                ann = self.parse_annotation(attr.spelling)
                break
        
        if ann or gather_members:
            data = {
                "id": len(members),
                "name": node.spelling,
                "type": node.type.spelling,
                "custom": field_ann,
                "ann": ann,
                "cindex": node
            }

            members.append(data)

    def has_meta_info(self, node):
        if node.location.file and str(node.location.file).endswith(self.file):
            return True
        return False
        
    def process_struct(self, node, depth):
        members = []
        methods = []
        ann = self.annotations.pop() if self.annotations else None
        custom = self.custom_annotation.pop() if self.custom_annotation else None
        gather_members = self.has_meta_info(node)
        namespace = "::".join(node.get_usr().split("@S@")[1:])
        struct = {
            "name": namespace,
            "spelling": node.spelling,
            "members": members,
            "ann": ann,
            "custom": custom,
            "methods": methods,
            "cindex": node,
        }

        # fields = [f.spelling for f in node.get_definition().get_fields()]
        # if fields:
        #     print(fields)

        field_ann = None
        for child in node.get_children():
            if node.spelling == "kmeta_annotation" and node.kind == clang.cindex.CursorKind.FUNCTION_DECL:
                # print(" " * depth, "Found custom kmeta_annotation")
                field_ann = child
                continue
        
            elif child.kind == clang.cindex.CursorKind.FIELD_DECL:
                self.add_field(members, child, gather_members, field_ann)
                
            elif child.kind == clang.cindex.CursorKind.CXX_METHOD:
                self.add_field(methods, child, gather_members, field_ann)

            elif child.kind == clang.cindex.CursorKind.ANNOTATE_ATTR:
                struct["ann"] = self.parse_annotation(child.spelling)

            else:
                self.find_class_struct_members(child, depth + 1)

        if ann or gather_members:
            if members or methods:
                self.structs[namespace] = struct

    def find_class_struct_members(self, node, depth):
        """
        Recursively find and print members of classes and structs in the AST nodes.
        """    
        if node.kind in [clang.cindex.CursorKind.CLASS_DECL, clang.cindex.CursorKind.STRUCT_DECL]:
            # print(" " * depth, f"Processing {node.spelling}")
            self.process_struct(node, depth)
            
        elif node.spelling == "kmeta_annotation" and node.kind == clang.cindex.CursorKind.FUNCTION_DECL:
            # print(" " * depth, "Found custom kmeta_annotation")
            self.custom_annotation.append(node)
        
        else:     
            for child in node.get_children():
                self.find_class_struct_members(child, depth + 1)



class KiwiPreprocessor(Preprocessor):
    def __init__(self, folder, file=None) -> None:
        self.options = {
            "exhaustive": 1,     # Reflect all properties by default even if not annotated
            "reflected": 1,      # Annotated properties are reflected by default
        }
        super().__init__(folder, file)
        
    def parse_annotation(self, ann):
        args = [arg.strip() for arg in ann.split(",")]

        annotation_kind = "unknown"
        if len(args) > 0:
            annotation_kind = args[0]

        annotation = {
            "kind": annotation_kind
        }

        for value in args[1:]:
            splits = value.split("=", maxsplit=1)
            if len(splits) == 2:
                annotation[splits[0]] = splits[1]
            else:
                annotation[value] = 1

        return annotation
    
    def flag(self, ann, key, type=int):
        return type(ann.get(key, self.options.get(key)))

    def reflected(self, ann):
        if ann is None:
            return self.options["exhaustive"]
        return self.flag(ann, "reflected", int)

    def reflect_property(self, attr) -> bool:
        return self.reflected(attr["ann"]) 
    
    def reflect_method(self, method) -> bool:
        return self.reflected(method["ann"]) 
    
    def reflect_type(self, struct) -> bool:
        return self.reflected(struct["ann"]) 


if __name__ == "__main__":
    import platform
    
    if platform.uname().system != "Linux":
        folder = "K:/lython/src"
        file = "K:/lython/lython/meta.h"
    else:
        folder = "/home/newton/work/lython/src"
        file = "/home/newton/work/lython/lython/meta.h"
    
    KiwiPreprocessor(folder, file)