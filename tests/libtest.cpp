#include "libtest.h"
#include "utilities/printing.h"
#include "utilities/strings.h"

#include <catch2/catch_all.hpp>
#include <fstream>
#include <iostream>
#include <regex>

namespace lython {

void write_case_v2(std::ostream& out, int i, VMTestCase const& testcase) {
    std::ostream& out2 = out; // std::cout;

    if (out.tellp() == 0) {
        out2 << "# version=2\n";
    }

    //if (testcase.name != "") 
    {
        out2 << "# > " << testcase.name << "\n";

        if (testcase.values.size() > 0) { 
            for(Section const& sect: testcase.values) {
                for(String const&val: sect.content) {
                    out2 << "# >> " << sect.name << "\n";
                    out2 << val;
                    out2 << "# <<\n\n";
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

Array<VMTestCase> load_cases_v2(std::istream& in) {
    std::regex start_regex("# > (.*)");
    std::regex more_regex("# >> (.*)");
    std::regex more_end_regex("(.*)# <<"); //  out << "# <<\n\n";
    Array<VMTestCase> cases;
    Array<String> buffer;
    std::string line;

    int case_idx = -1;
    int section_idx = -1;
    auto close_previous = [&](){
        if (buffer.size() > 0) {
            
            String str = join("\n", buffer);
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

    while (std::getline(in, line)) {
        std::smatch match;
        if (std::regex_match(line, match, start_regex)) {
            new_case(match[1].str());
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

Array<VMTestCase> load_cases(std::istream& in) {
    std::regex start_regex("# version=(.*)");
    std::string line;
    std::getline(in, line);
    std::smatch match;    

    if (std::regex_match(line, match, start_regex)) {
        std::string version = match[1].str();

        if (version == "2") {
            std::cout << "using v2\n";
            return load_cases_v2(in);
        }
    }
    
    std::cout << "using v1\n";
    in.seekg(0, in.beg);
    return load_cases_v1(in);
}


String reg_modules_path() { return String(_SOURCE_DIRECTORY) + "/tests/cases"; }

Array<VMTestCase> get_test_cases(String const& folder, String const& name, Array<VMTestCase> const& maybe_cases) {
    return transition(folder, name, maybe_cases);
}

Array<VMTestCase> get_test_cases(String const& folder, String const& name) {
    String path = (reg_modules_path() + String("/") + folder + String("/") + name + String(".py"));

    std::ifstream     testfile_fp(path.c_str());
    Array<VMTestCase> cases = load_cases(testfile_fp);
    return cases;
}

Array<VMTestCase> transition(String const& folder, String const& name, Array<VMTestCase> const& cases) {
    String path = (reg_modules_path() + String("/") + folder + String("/") + name + String(".py"));

    {
        #if 0
        {
            std::ofstream fout(path.c_str());
            int           i = 0;
            for (auto& c: cases) {
                write_case(fout, i, c);
                i += 1;
            }
        }
        #endif

        // load possibly old version
        Array<VMTestCase> loaded_cases;
        {
            std::ifstream fin(path.c_str());
            loaded_cases = load_cases(fin);
            
            if (cases.size() == 0) {
                return loaded_cases;
            }
        }

        // write new version
        {
            std::ofstream fout((path).c_str());
            int           i = 0;
            for (auto& c: loaded_cases) {
                write_case(fout, i, c);
                i += 1;
            }
        }

        REQUIRE(loaded_cases.size() == cases.size());
        for (int i = 0; i < cases.size(); i++) {
            VMTestCase const& original = cases[i];
            VMTestCase const& loaded   = loaded_cases[i];

            REQUIRE(original.code == loaded.code);
            REQUIRE(original.call == loaded.call);
            REQUIRE(original.expected_type == loaded.expected_type);
            REQUIRE(original.errors == loaded.errors);

            if (!original.name.empty()) {
                REQUIRE(original.name == loaded.name);
            }
        }
    }

    return cases;
}

}  // namespace lython