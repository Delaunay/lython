#include "libtest.h"
#include "utilities/printing.h"
#include "utilities/strings.h"

#include <cstdio>
#include <catch2/catch_all.hpp>
#include <fstream>
#include <iostream>
#include <regex>
#include <filesystem>

namespace lython {

template<typename T, typename ...Args>
bool contains(T value, Args... args) {
    std::vector<T> values{args...};
    for(T const& val: values) {
        if (val == value) {
            return true;
        }
    }
    return false;
}

void write_case_v2(std::ostream& out, int i, VMTestCase const& testcase) {
    std::ostream& out2 = out; // std::cout;

    if (out.tellp() == 0) {
        out2 << "# version=2\n";
    }

    auto long_format = [&](String const& section, String const& content) {
        out2 << "# >> " << section << "\n";
        out2 << content;
        out2 << "# <<\n\n";
    };

    auto short_format = [&](String const& section, String const& content) {
        out2 << "# >> " << section << ":: " << content << "\n";
    };

    auto both_format = [&](String const& section, String const& content)  {
        if (content.find_first_of('\n') != std::string::npos) {
            return long_format(section, content);
        }
        return short_format(section, content);
    };

    /*
    Dict<String, std::function<void(String const&, String const&)>> dispatch = {
        {"code", long_format},
        {"call", long_format},
        {"expected": long_format},
    }; */
    //if (testcase.name != "") 
    {
        out2 << "# > " << testcase.name << "\n";

        if (testcase.values.size() > 0) { 
            for(Section const& sect: testcase.values) {
                for(String const&val: sect.content) {
                    if (contains(sect.name, "error", "type", "call", "result")) {
                        both_format(sect.name, val);
                        continue;
                    }
                    long_format(sect.name, val);
                }
            }
        } else {
            out2 << "# >> code\n";
            out2 << testcase.code;
            out2 << "# <<\n";
            out2 << "\n";

            if (!testcase.call.empty()) {
                out2 << "# >> call\n";
                out2 << testcase.call;
                out2 << "# <<\n";
                out2 << "\n";
            }

            if (!testcase.expected_type.empty()) {
                out2 << "# >> expected\n";
                out2 << testcase.expected_type;
                out2 << "# <<\n";
                out2 << "\n\n";
            }

            if (!testcase.errors.empty()) {
                for (auto& error: testcase.errors) {
                    out2 << "# >> error\n";
                    out2 << error;
                    out2 << "# <<\n";
                }
                out2 << "\n";
            }
        }
    }
}

void write_case_v1(std::ostream& out, int i, VMTestCase const& testcase) {
    if (out.tellp() == 0) {
        out << "# version=1\n";
    }

    if (testcase.name.empty()) {
        out << "# >>> case: " << i << "\n";
    } else {
        out << "# >>> case: " << testcase.name << "\n";
    }
    out << "# >>> code\n";
    out << testcase.code;
    out << "# <<<\n";
    out << "\n";

    if (!testcase.call.empty()) {
        out << "# >>> call\n";
        out << testcase.call;
        out << "# <<<\n";
        out << "\n";
    }

    if (!testcase.expected_type.empty()) {
        out << "# >>> expected\n";
        out << testcase.expected_type;
        out << "# <<<\n";
        out << "\n\n";
    }

    if (!testcase.errors.empty()) {
        for (auto& error: testcase.errors) {
            out << "# >>> error\n";
            out << error;
            out << "# <<<\n";
        }
        out << "\n";
    }
}

void write_case(std::ostream& out, int i, VMTestCase const& testcase) {
    write_case_v2(out, i, testcase);
}

enum class CaseSection
{
    None,
    Case,
    Code,
    Call,
    Expected,
    Error
};


struct LineReader {
    LineReader(const char* filename) {
        _file = fopen(filename, "r");
        finished = false;
    }

    ~LineReader() {
        fflush(_file);
        fclose(_file);
    }

    std::string const& readline() {
        line.resize(0);
        while (true) {
            int c = fgetc(_file);
            if (c == EOF) {
                finished = true;
                return line;
            }
            line.push_back(c);
            if (c == '\n') {
                return line;
            }
        }
        return line;
    }

    operator bool() const {
        return !finished;
    }

    bool finished;
    std::string line;
    FILE* _file = nullptr;
};


Array<VMTestCase> load_cases_v2(const char* path) {
    #define NEWLINE "(\r?\n)"
    //#undef NEWLINE
    //#define NEWLINE ""
    std::regex start_regex("^# > (.*)" NEWLINE);
    std::regex inline_regex("^# >> (.*):: (.*)" NEWLINE);
    std::regex more_regex("^# >> (.*)" NEWLINE);
    std::regex more_end_regex("^(.*)# <<" NEWLINE); //  out << "# <<\n\n";
    Array<VMTestCase> cases;
    Array<String> buffer;
    std::string line;

    int case_idx = -1;
    int section_idx = -1;
    int empty_line = 0;

    auto close_previous = [&](){
        if (buffer.size() > 0) {
            String str = join("", buffer);
            buffer.resize(0);

            if (section_idx >= 0 && case_idx >= 0) {
                VMTestCase& current = cases[case_idx];
                Section& section = current.values[section_idx];
                section.content.push_back(str);
            }
        }

        section_idx = -1;
    };

    auto new_case = [&](std::string const& name) {
        close_previous();
        buffer.resize(0);

        VMTestCase newcase{"", ""};
        newcase.version = 2;
        newcase.name = name.c_str();
        section_idx = -1;
        case_idx = cases.size();
        cases.push_back(newcase);
    };

    auto new_section = [&](String const& section_name) {
        close_previous();

        VMTestCase& current = cases[case_idx];
        int i = 0;
        section_idx = -1;
        for(Section& sec: current.values) {
            if (sec.name == section_name) {
                section_idx = i;
                break;
            }
            i += 1;
        }
        if (section_idx == -1) {
            Section section;
            section.name = section_name;
            section_idx = current.values.size();
            current.values.push_back(section);
        }
    };

    LineReader reader(path);

    while (reader) {
        line = reader.readline();

        std::smatch match;
        if (std::regex_match(line, match, start_regex)) {
            new_case(match[1].str());
            continue;
        }
        if (std::regex_match(line, match, inline_regex)) {
            new_section(match[1].str().c_str());
            buffer.push_back(String(match[2].str().c_str()));
            close_previous();
            continue;
        }
        if (std::regex_match(line, match, more_regex)) {
            new_section(match[1].str().c_str());
            continue;
        }
        if (std::regex_match(line, match, more_end_regex)) {
            buffer.push_back(String(match[1].str().c_str()));
            close_previous();
            continue;
        }
        buffer.push_back(str(line));
    }

    return cases;
}

Array<VMTestCase> load_cases_v1(std::istream& in) {
    std::regex case_regex("# >>> case: (.*)");
    std::regex code_regex("# >>> code");
    std::regex call_regex("# >>> call");
    std::regex expe_regex("# >>> expected");
    std::regex error_regex("# >>> error");
    std::regex content_regex("(.*)# <<<", std::regex::extended);

    Array<String> buffer;
    Array<VMTestCase> cases;

    int         i = 0;
    VMTestCase  currentcase{"", ""};
    CaseSection section = CaseSection::None;
    std::string line;

    auto add_case = [&]() {
        if (i > 0) {
            cases.push_back(currentcase);
            currentcase = VMTestCase("", "");
            currentcase.version = 1;
            i           = 0;
        }
    };

    auto push = [&]() {
        if (buffer.size() > 0) {
            String str = join("\n", buffer);
            buffer.resize(0);

            switch (section) {
            case CaseSection::Case: {
                add_case();
                return;
            }
            case CaseSection::Code: {
                currentcase.code = str;
                i += 1;
                return;
            }
            case CaseSection::Call: {
                currentcase.call = str;
                i += 1;
                return;
            }
            case CaseSection::Error: {
                currentcase.errors.push_back(str);
                i += 1;
                return;
            }
            case CaseSection::Expected: {
                currentcase.expected_type = str;
                i += 1;
                return;
            }
            case CaseSection::None: {
                return;
            }
            }
        }
    };

    while (std::getline(in, line)) {
        std::smatch match;
        if (std::regex_match(line, match, case_regex)) {
            push();
            add_case();
            currentcase.name = match[1].str();
            section          = CaseSection::Case;
            continue;
        }
        if (std::regex_search(line, code_regex)) {
            push();
            section = CaseSection::Code;
            continue;
        }
        if (std::regex_search(line, call_regex)) {
            push();
            section = CaseSection::Call;
            continue;
        }
        if (std::regex_search(line, expe_regex)) {
            push();
            section = CaseSection::Expected;
            continue;
        }
        if (std::regex_search(line, error_regex)) {
            push();
            section = CaseSection::Error;
            continue;
        }

        if (std::regex_match(line, match, content_regex)) {
            buffer.push_back(str(match[1].str()));
            push();
            section = CaseSection::None;
            continue;
        }

        buffer.push_back(str(line));
    }
    push();
    add_case();

    // kwassert(cases.size() > 0, "Should have cases");
    std::cout << cases.size() << std::endl;
    return cases;
}

Array<VMTestCase> load_cases(const char* path, std::istream& in) {
    std::regex start_regex("# version=(.*)");
    std::string line;
    std::getline(in, line);
    std::smatch match;    

    if (std::regex_match(line, match, start_regex)) {
        std::string version = match[1].str();

        if (version == "2") {
            std::cout << "using v2\n";
            return load_cases_v2(path);
        }
    }
    
    std::cout << "using v1\n";
    in.seekg(0, in.beg);
    Array<VMTestCase> old_cases = load_cases_v1(in);
    
    {
        std::ofstream fout(path);
        int           i = 0;
        for (auto& c: old_cases) {
            write_case(fout, i, c);
            i += 1;
        }
        fout.flush();
        fout.close();
    }


    return old_cases;
}


String reg_modules_path() { return String(_SOURCE_DIRECTORY) + "/tests/cases"; }

Array<VMTestCase> get_test_cases(String const& folder, String const& name, Array<VMTestCase> const& maybe_cases) {
    return transition(folder, name, maybe_cases);
}

Array<VMTestCase> get_test_cases(String const& folder, String const& name) {
    String path = (reg_modules_path() + String("/") + folder + String("/") + name + String(".py"));

    std::ifstream     testfile_fp(path.c_str());
    Array<VMTestCase> cases = load_cases(path.c_str(), testfile_fp);
    return cases;
}


template<typename T>
int ensure(String const& folder, String const& name, T const& A, T const& B) {
    if (A != B) {
        std::cout << "    >" << A << "< \n\n";
        std::cout << "    >" << B << "< \n";
        return 1;
    }
    REQUIRE(A == B);
    return 0;
}

Array<VMTestCase> transition(String const& folder, String const& name, Array<VMTestCase> const& cases) {
    String path = (reg_modules_path() + String("/") + folder + String("/") + name + String(".py"));

    bool cases_loaded = false;
    Array<VMTestCase> loaded_cases;

    std::size_t size = 0;
    if (std::filesystem::exists(path)) {
        size = std::filesystem::file_size(path);
    }
    bool file_existed = std::filesystem::exists(path) && size != 0;

    std::cout << size << " " << path << "\n";

    // Test case does not exist create it now
    if (!file_existed)
    {
        std::ofstream fout(path.c_str());
        int           i = 0;
        for (auto& c: cases) {
            write_case(fout, i, c);
            i += 1;
        }
        fout.flush();
        fout.close();
    }
    else {
        // load the test case from the files
        std::ifstream fin(path.c_str());
        loaded_cases = load_cases(path.c_str(), fin);
        fin.close();
        cases_loaded = true;
    }

    if (cases_loaded && cases.size() > 0) {
        int errors = ensure(folder, name, loaded_cases.size(), cases.size());

        int size = std::min(cases.size(), loaded_cases.size());

        for (int i = 0; i < size; i++) {
            std::cout << "\n\n";
            std::cout << folder << " " << name  << ": " << i << "\n";
            std::cout << String(80, '=') << "\n";

            VMTestCase const& original = cases[i];
            VMTestCase const& loaded   = loaded_cases[i];

            if (loaded.version == 2) {
                if (original.code.size() != 0) {
                    errors += ensure(folder, name, original.code, loaded.get_code());
                }
                if (original.call.size() != 0) {
                    errors += ensure(folder, name, original.call, loaded.get_call());
                }
                if (original.expected_type.size() != 0) {
                    errors += ensure(folder, name, original.expected_type, loaded.get_expected_type());
                }
                if (original.errors.size() != 0) {
                    errors += ensure(folder, name, original.errors, loaded.get_errors());
                }
                if (!original.name.empty()) {
                    errors += ensure(folder, name, original.name, loaded.name);
                }
            } 
            
            if (loaded.version == 1) {
                errors += ensure(folder, name, original.code, loaded.code);
                errors += ensure(folder, name, original.call, loaded.call);
                errors += ensure(folder, name, original.expected_type, loaded.expected_type);
                errors += ensure(folder, name, original.errors, loaded.errors);

                if (!original.name.empty()) {
                    errors += ensure(folder, name, original.name, loaded.name);
                }
            }

            if (errors > 0) {
                for (int i = 0; i < loaded_cases.size(); i++) {
                    VMTestCase const& lc = loaded_cases[i];
                    if (lc.version == 2) {
                        for (auto& sec: lc.values) {
                            std::cout << "    `" << sec.name << "` " << sec.name.size() << std::endl;
                            std::cout << "    `" << sec.content << "`" << std::endl;
                        }
                    }
                }
            }
        }
    }

    // write new version
    if (file_existed) {
        std::ofstream fout((path).c_str());
        int           i = 0;
        for (auto& c: loaded_cases) {
            write_case(fout, i, c);
            i += 1;
        }
        fout.flush();
        fout.close();
    }

    return cases;
}

}

#include "sema/sema.h"

namespace lython {

String NE(String const& name) { return String(NameError(nullptr, name).message().c_str()); }

String NC(std::string const& name) {
    return String(fmt::format("{} is not callable", name).c_str());
}

String TE(String const& lhs_v, String const& lhs_t, String const& rhs_v, String const& rhs_t) {
    return String(TypeError::message(lhs_v, lhs_t, rhs_v, rhs_t));
}

String AE(String const& name, String const& attr) {
    return String(AttributeError::message(name, attr));
}

String UO(String const& op, String const& lhs, String const& rhs) {
    return String(UnsupportedOperand::message(op, lhs, rhs));
}

String IE(String const& module, String const& name) {
    return String(ImportError::message(module, name));
}

String MNFE(String const& module) { return String(ModuleNotFoundError::message(module)); }


}  // namespace lython


//
//
#include "libtest_generator.h"

namespace lython {
//
//
FileTestCaseGenerator::FileTestCaseGenerator(String const& folder, String const& name) {
    path = (reg_modules_path() + String("/") + folder + String("/") + name + String(".py"));
    cases = get_test_cases(folder, name);
}

TestCase const& FileTestCaseGenerator::get() const {
    return cases[i];
}

bool FileTestCaseGenerator::next() {
    i += 1;    
    return i < cases.size();
}

Catch::Generators::GeneratorWrapper<TestCase> filecase(String const& folder, String const& name) {
    return Catch::Generators::GeneratorWrapper<TestCase>(
        new FileTestCaseGenerator(folder, name)
    );
}


Tuple<Array<VMTestCase>, Array<std::filesystem::path>> load_recursive_cases(String const& folder) {
    Array<VMTestCase> cases;
    Array<std::filesystem::path> folders;

    String path = reg_modules_path() + String("/") + folder;

    for(auto item: std::filesystem::recursive_directory_iterator(path)) {
        if (item.is_regular_file()) {
            const char* path = item.path().string().c_str();

            std::ifstream     testfile_fp(path);
            Array<VMTestCase> newcases = load_cases(path, testfile_fp);
            cases.insert(cases.end(), newcases.begin(), newcases.end());

            folders.push_back(item);
        }
    }
    return std::make_tuple(cases, folders);
}

//
//
FolderTestCaseGenerator::FolderTestCaseGenerator(String const& folder) {
    std::tie(cases, folders) = load_recursive_cases(folder);
}

TestCase const& FolderTestCaseGenerator::get() const {
    return cases[i];
}

bool FolderTestCaseGenerator::next() {
    i += 1;    
    return i < cases.size();
}

Catch::Generators::GeneratorWrapper<TestCase> filecase(String const& folder) {
    return Catch::Generators::GeneratorWrapper<TestCase>(
        new FolderTestCaseGenerator(folder)
    );
}

//
//
AllTestCaseGenerator::AllTestCaseGenerator() {
   std::tie(cases, folders) = load_recursive_cases(".");
}

TestCase const& AllTestCaseGenerator::get() const {
    return cases[i];
}

bool AllTestCaseGenerator::next() {
    i += 1;    
    return i < cases.size();
}

Catch::Generators::GeneratorWrapper<TestCase> filecase() {
    return Catch::Generators::GeneratorWrapper<TestCase>(
        new AllTestCaseGenerator()
    );
}

std::string make_name(TestCase const& name, int i, std::filesystem::path path) {
    std::stringstream ss;
    ss << path.lexically_relative(reg_modules_path());
    ss << " ";
    ss << name.name;
    ss << " " << i;
    return ss.str();
}

std::string FileTestCaseGenerator::stringifyImpl() const {
    return make_name(get(), i, path);
}
std::string FolderTestCaseGenerator::stringifyImpl() const {
    return make_name(get(), i, folders[i]);
}
std::string AllTestCaseGenerator::stringifyImpl() const {
    return make_name(get(), i, folders[i]);
}
}