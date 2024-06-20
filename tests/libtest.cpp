#include "libtest.h"
#include "utilities/printing.h"
#include "utilities/strings.h"

#include <catch2/catch_all.hpp>
#include <fstream>
#include <iostream>
#include <regex>

namespace lython {

void write_case(std::ostream& out, int i, VMTestCase const& testcase) {

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

enum class CaseSection
{
    None,
    Case,
    Code,
    Call,
    Expected,
    Error
};

Array<VMTestCase> load_cases(std::istream& in) {
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
        {
            std::ofstream fout(path.c_str());
            int           i = 0;
            for (auto& c: cases) {
                write_case(fout, i, c);
                i += 1;
            }
        }

        Array<VMTestCase> loaded_cases;
        {
            std::ifstream fin(path.c_str());
            loaded_cases = load_cases(fin);
            
            if (cases.size() == 0) {
                return loaded_cases;
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