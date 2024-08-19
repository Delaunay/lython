import os
from collections import defaultdict
from pathlib import Path

import clang.cindex


class Preprocessor:
    def __init__(self, folder, output) -> None:
        self.index = clang.cindex.Index.create()
        self.tu = []
        self.output = output
        self.folder = folder
        self.ignored = defaultdict(int)
        self.annotations = []
        self.structs = dict()
        self.custom_annotation = []
        self.macros = {"KWMETA"}

        self.history = []
        self.files = None
        self.parse_files(folder)
        
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

    def parse_files(self, folder_or_file):
        if os.path.isfile(folder_or_file):
            self.files = [folder_or_file]
        else:
            self.files = list(Path(folder_or_file).glob("**/*.h"))

        for f in self.files:
            if "old" in str(f):
                continue
            
            # print(f)
            args = [
                "-x",
                "c++",
                "-std=c++20",
                "-nostdinc",
                # "-I", str(folder_or_file),
                "-I", "K:/lython/src",
                "-I", "/home/newton/work/lython/src",
                "-DKMETA_PROCESSING=1",
            ]
            tu = self.index.parse(
                str(f),
                args=args,
                options=clang.cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD
                | clang.cindex.TranslationUnit.PARSE_INCOMPLETE
                | clang.cindex.TranslationUnit.PARSE_SKIP_FUNCTION_BODIES,
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
        return
        # Check for parsing errors
        for diag in tu.diagnostics:
            print(f"Diagnostic: {diag.severity} {diag.spelling}")
            if diag.severity == clang.cindex.Diagnostic.Error:
                print("HTError:", diag.spelling)
            elif diag.severity == clang.cindex.Diagnostic.Warning:
                print("HTWarning:", diag.spelling)
            elif diag.severity == clang.cindex.Diagnostic.Note:
                print("HTNote:", diag.spelling)
            elif diag.severity == clang.cindex.Diagnostic.Fatal:
                print("HTFatal error:", diag.spelling)

    def generate_data(self):
        includes = set()

        def included_file(node):
            try:
                cursor = node["cindex"]
                for i in cursor.translation_unit.get_includes():
                    if str(i.source) not in includes:
                        includes.add(str(i.source))
                        print(f'#include "{i.source}"', file=fp)

            except Exception as err :
                print("ERROR:", err)

        
        with open(os.path.join(self.folder, self.output), "w") as fp:

            for f in self.files:
                if "old" in str(f):
                    continue

                print(f'#include "{f}"', file=fp)

            #for name, struct in self.structs.items():
            #    included_file(struct)

            print('#include "dtypes.h"', file=fp)
            print('#include "ast/nodes.h"', file=fp)
            print('#include "utilities/names.h"', file=fp)
            print("", file=fp)

            for name, struct in self.structs.items():
                if self.reflect_type(struct):
                    if len(struct["members"]) == 0:
                        continue
                
                    self.struct_begin(struct, fp)

                    for attr in struct["members"]:
                        if self.reflect_property(attr):
                            self.generate_proprety(struct, attr, fp)

                    for attr in struct["methods"]:
                        if self.reflect_method(attr):
                            self.generate_method(struct, attr, fp)

                    self.struct_end(struct, fp)

    def struct_begin(self, struct, fp):
        typename = self.get_typename(struct)

        print(f"template <>", file=fp)
        print(f"struct lython::meta::ReflectionTrait<{typename}> {{", file=fp)
        print(f"    static int register_members() {{", file=fp)

    def struct_end(self, struct, fp):
        print(f"        return 1;", file=fp)
        print(f"    }}", file=fp)
        print(f"}};", file=fp)

    def generate_proprety(self, struct, attr, fp):
        attrname = attr["name"]
        typename = self.get_typename(struct)

        print(
            f'        lython::meta::register_member<&{typename}::{attrname}>("{attrname}");',
            file=fp,
        )

    def get_typename(self, struct):
        return "::".join([*struct["namespaces"], struct["demangle"]])

    def generate_method(self, struct, attr, fp):
        attrname = attr["name"]
        cindex = attr["cindex"]
        type = attr["type"]
        typename = self.get_typename(struct)

        if cindex.is_static_method():
            return
        
        ann = attr.get("ann", {}) or {}
        overriden_type = ann.get("type", None)
        
        
        
        if overriden_type:
            if attrname.startswith("operator"):
                ops = {
                    "==": "operator_equal",
                    "!=": "operator_notequal"
                }
                op = attrname.replace("operator", "")
                name = ops.get(op, "Unhandled")
            else:
                name = attrname
                    
            print(
                f'        using {name}T = {overriden_type};\n'
                f'        lython::meta::register_member<static_cast<{name}T>(&{typename}::{attrname})>("{attrname}");',
                file=fp,
            )

        else:
            # print(
            #     f'        //{type}',
            #     file=fp,
            # )
            print(
                f'        lython::meta::register_member<&{typename}::{attrname}>("{attrname}");',
                file=fp,
            )

    def parse_annotation(self, ann):
        return ann

    def add_field(self, members, node: clang.cindex.Cursor, gather_members, field_ann):
        ann = None
        for attr in node.get_children():
            if attr.kind == clang.cindex.CursorKind.ANNOTATE_ATTR:
                ann = self.parse_annotation(attr.spelling)
                break
        
        if node.access_specifier != clang.cindex.AccessSpecifier.PUBLIC:
            return

        # print(node.availability)
        # print(node.access_specifier)
        data = {
            "id": len(members),
            "name": node.spelling,
            "type": node.type.spelling,
            "custom": field_ann,
            "ann": ann,
            "cindex": node,
        }

        if node.spelling == "Array":
            print(node.type.spelling, ann, node.spelling, node.get_usr())

        members.append(data)

        # if node.spelling == "write":
        #    print(members)

    def has_meta_info(self, node):
        # if node.location.file and str(node.location.file).endswith(self.file):
        #    return True
        return False

    def accept_path(self, p):
        try:
            return str(p).startswith(str(self.folder))
            print(os.path.commonpath([str(p), self.folder]))
            return True
        except:
            return False
    

    def process_struct(self, node, depth):
        def demangled():
            return node.displayname

        if not self.accept_path(node.location.file):
            return
     
        members = []
        methods = []
        ann = self.annotations.pop() if self.annotations else None
        custom = self.custom_annotation.pop() if self.custom_annotation else None
        gather_members = self.has_meta_info(node)
        namespace = "::".join(node.get_usr().split("@S@")[1:])

        prefix = "c:@N@"
        usr = node.get_usr()
        usr = usr.removeprefix(prefix).split(">#*F$")[0]
        namespaces = []
        for frag in usr.split("@S@"):
            namespaces.extend(frag.split("@N@"))
        
        struct = {
            "namespaces": namespaces[:-1],
            "name": namespaces[-1],
            "demangle": demangled(),
            "spelling": node.spelling,
            "members": members,
            "ann": ann,
            "custom": custom,
            "methods": methods,
            "cindex": node,
        }

        field_ann = None
        for child in node.get_children():
            if (
                node.spelling == "kmeta_annotation"
                and node.kind == clang.cindex.CursorKind.FUNCTION_DECL
            ):
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

        if struct["name"] == "_LyException":
            print(struct)

        if members or methods:
            self.structs[namespace] = struct

    def find_class_struct_members(self, node, depth):
        """
        Recursively find and print members of classes and structs in the AST nodes.
        """
        if node.kind in [
            clang.cindex.CursorKind.CLASS_DECL,
            clang.cindex.CursorKind.STRUCT_DECL,
        ]:
            # print(" " * depth, f"Processing {node.spelling}")
            self.process_struct(node, depth)

        elif (
            node.spelling == "kmeta_annotation"
            and node.kind == clang.cindex.CursorKind.FUNCTION_DECL
        ):
            # print(" " * depth, "Found custom kmeta_annotation")
            self.custom_annotation.append(node)

        else:
            for child in node.get_children():
                self.find_class_struct_members(child, depth + 1)


class KiwiPreprocessor(Preprocessor):
    def __init__(self, *args, **kwargs) -> None:
        self.options = {
            "exhaustive": 1,  # Reflect all properties by default even if not annotated
            "reflected": 1,  # Annotated properties are reflected by default
        }
        super().__init__(*args, **kwargs)

    def parse_annotation(self, ann):
        args = [arg.strip() for arg in ann.split(",")]

        annotation_kind = "unknown"
        if len(args) > 0:
            annotation_kind = args[0]

        annotation = {"kind": annotation_kind}

        for value in args[1:]:
            splits = value.split("=", maxsplit=1)
            if len(splits) == 2:
                annotation[splits[0]] = splits[1]
            else:
                annotation[value] = 1

        return annotation

    def flag(self, ann, key, type=int):
        return type(ann.get(key, self.options.get(key)))

    def _reflected(self, ann):
        if ann is None:
            return self.options["exhaustive"]
        return self.flag(ann, "reflected", int)

    def reflected(self, ann):
        r = self._reflected(ann)
        return r

    def reflect_property(self, attr) -> bool:
        return self.reflected(attr["ann"])

    def reflect_method(self, method) -> bool:
        return self.reflected(method["ann"])

    def reflect_type(self, struct) -> bool:
        if struct["name"] == "Output":
            print(struct)
        
        return self.reflected(struct["ann"])


from dataclasses import dataclass


@dataclass
class Arguments:
    source: str
    output: str


def main():
    import os

    from argklass import ArgumentParser

    parser = ArgumentParser()
    parser.add_arguments(Arguments)
    args = parser.parse_args()

    args.output = os.path.join(os.getcwd(), args.output)
    print("Writing to:", args.output)
    KiwiPreprocessor(args.source, args.output)

    print("DONE")


def test():
    import platform

    if platform.uname().system != "Linux":
        folder = "K:/lython/src"
        file = "K:/lython/lython/meta.h"
    else:
        folder = "/home/newton/work/lython/src"
        file = "/home/newton/work/lython/lython/meta.h"

    KiwiPreprocessor(folder, file)


if __name__ == "__main__":
    main()
