import os
from collections import defaultdict
from pathlib import Path

import clang.cindex



class Project:
    def __init__(self, folder, file=None) -> None:
        self.index = clang.cindex.Index.create() 
        self.tu = []
        self.folder = folder
        self.ignored = defaultdict(int)
        self.annotations = []
        self.structs = dict()
        self.macros = {
            "KWMETA"
        }
        
        self.parse_files(folder, file)
        for tu in self.tu:
            print(tu)
            self.find_class_struct_members(tu.cursor, 0)
        self.generate_data()

    def parse_files(self, folder, file=None):
        files = [file] if file else list(Path(folder).glob('**/*.h'))
        for f in files:
            args = ['-x', 'c++', '-std=c++20',  "-nostdinc", "-P"] 
            tu = self.index.parse(
                str(f), 
                args=args, 
                options=clang.cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD
            )
            self.tu.append(tu)
            self.check_for_errors(tu)

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
        for name, members in self.structs.items():
            print(name, members)

    def parse_annotation(self, ann):
        return ann

    def add_field(self, members, node):
        ann = None
        for attr in node.get_children():
            if attr.kind == clang.cindex.CursorKind.ANNOTATE_ATTR:
                ann = self.parse_annotation(attr.spelling)
                break
        
        if ann:
            members.append({
                "id": len(members),
                "name": node.spelling,
                "type": node.type.spelling,
                "ann": ann,
            })
        
    def process_struct(self, node, depth):
        members = []
        methods = []
        ann = self.annotations.pop() if self.annotations else None
        struct = {
            "name": node.spelling,
            "ann": ann,
            "members": members,
            "methods": methods
        }
        for child in node.get_children():
            # print("." * depth, node.spelling, child.kind, child.spelling)
            if child.kind == clang.cindex.CursorKind.FIELD_DECL:
                self.add_field(members, child)
                
            elif child.kind == clang.cindex.CursorKind.CXX_METHOD:
                self.add_field(methods, child)

        if members or methods or ann:
            self.structs[node.spelling] = struct

    def find_class_struct_members(self, node, depth):
        """
        Recursively find and print members of classes and structs in the AST nodes.
        """
        if node.kind not in (clang.cindex.CursorKind.MACRO_DEFINITION, clang.cindex.CursorKind.INCLUSION_DIRECTIVE):
            print("." * depth, node.kind, node.spelling)

        if node.location.file and (not str(node.location.file).startswith(self.folder)):
            # print("Skipping...", node.location.file)
            self.ignored[str(node.location.file)] += 1
            return
        
        if node.kind == clang.cindex.CursorKind.ANNOTATE_ATTR:
            # print("ATTR", node.spelling)
            self.annotations.append(self.parse_annotation(node.spelling))
            
        elif node.kind in [clang.cindex.CursorKind.CLASS_DECL, clang.cindex.CursorKind.STRUCT_DECL]:
            self.process_struct(node, depth)
        
        else:     
            for child in node.get_children():
                self.find_class_struct_members(child, depth + 1)



if __name__ == "__main__":
    folder = "K:/lython/src"
    file = "K:/lython/src/ast/nodes.h"
    Project(folder, file)