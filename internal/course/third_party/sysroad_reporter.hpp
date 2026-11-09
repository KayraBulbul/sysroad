#pragma once
// sysroad's doctest reporter, compiled into the test runner's main(). It
// writes one JSON object per line and flushes each one, so the grader can
// lay the results out itself, and still knows which test case was running
// when a sanitizer kills the process mid-test. Only failures are reported in
// detail; passing asserts aren't written at all.
#include "doctest.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace sysroad_report {
using namespace doctest;

inline void escape(std::string& out, const char* s, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == '"' || c == '\\') {
            out += '\\';
            out += static_cast<char>(c);
        } else if (c < 0x20 || c == 0x7f) {
            char buf[8];
            std::snprintf(buf, sizeof buf, "\\u%04x", c);
            out += buf;
        } else {
            out += static_cast<char>(c);
        }
    }
}

inline std::string quoted(const char* s, std::size_t n) {
    std::string out = "\"";
    escape(out, s, n);
    return out + '"';
}
inline std::string quoted(const char* s) { return s ? quoted(s, std::strlen(s)) : "\"\""; }
inline std::string quoted(const String& s) { return quoted(s.c_str(), s.size()); }
inline std::string quoted(const std::string& s) { return quoted(s.c_str(), s.size()); }

inline std::string array(const std::vector<std::string>& items) {
    std::string out = "[";
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i > 0) out += ',';
        out += quoted(items[i]);
    }
    return out + ']';
}

struct Obj {
    std::string s = "{";
    Obj& raw(const char* k, const std::string& v) {
        if (s.size() > 1) s += ',';
        s += '"';
        s += k;
        s += "\":";
        s += v;
        return *this;
    }
    template <typename T> Obj& str(const char* k, const T& v) { return raw(k, quoted(v)); }
    Obj& num(const char* k, long long v) { return raw(k, std::to_string(v)); }
    Obj& flag(const char* k, bool v) { return raw(k, v ? "true" : "false"); }
};

struct Reporter : IReporter {
    std::ostream& out;
    std::vector<std::string> subcases;

    explicit Reporter(const ContextOptions& in) : out(*in.cout) {}

    void emit(const Obj& o) { out << o.s << "}\n" << std::flush; }

    // What INFO() and CAPTURE() hold right now.
    static std::vector<std::string> info() {
        std::vector<std::string> items;
        int n = IReporter::get_num_active_contexts();
        auto contexts = IReporter::get_active_contexts();
        for (int i = 0; i < n; ++i) {
            std::ostringstream ss;
            contexts[i]->stringify(&ss);
            items.push_back(ss.str());
        }
        return items;
    }

    void report_query(const QueryData&) override {}

    void test_run_start() override {
        std::vector<const detail::TestCase*> all;
        for (auto& tc : detail::getRegisteredTests()) all.push_back(&tc);
        std::sort(all.begin(), all.end(), [](const detail::TestCase* a, const detail::TestCase* b) {
            int c = a->m_file.compare(b->m_file);
            return c != 0 ? c < 0 : a->m_line < b->m_line;
        });
        for (auto tc : all)
            emit(Obj().str("ev", "case").str("name", tc->m_name).str("file", tc->m_file.c_str()).num("line", tc->m_line));
    }
    void test_run_end(const TestRunStats&) override { emit(Obj().str("ev", "done")); }

    void test_case_start(const TestCaseData& in) override {
        subcases.clear();
        emit(Obj().str("ev", "start").str("name", in.m_name).str("file", in.m_file.c_str()).num("line", in.m_line));
    }
    void test_case_reenter(const TestCaseData&) override { subcases.clear(); }
    void test_case_end(const CurrentTestCaseStats& st) override {
        emit(Obj().str("ev", "end").flag("ok", st.testCaseSuccess));
    }
    void test_case_exception(const TestCaseException& e) override {
        emit(Obj().str("ev", "exception").flag("crash", e.is_crash).str("text", e.error_string));
    }
    void subcase_start(const SubcaseSignature& s) override { subcases.push_back(s.m_name.c_str()); }
    void subcase_end() override {
        if (!subcases.empty()) subcases.pop_back();
    }

    void log_assert(const AssertData& a) override {
        if (!a.m_failed) return;
        emit(Obj()
                 .str("ev", "assert")
                 .str("type", assertString(a.m_at))
                 .str("file", a.m_file)
                 .num("line", a.m_line)
                 .str("expr", a.m_expr)
                 .str("decomp", a.m_decomp)
                 .flag("threw", a.m_threw)
                 .str("exception", a.m_exception)
                 .str("exc_type", a.m_exception_type)
                 .str("exc_string", a.m_exception_string.c_str())
                 .raw("subcases", array(subcases))
                 .raw("info", array(info())));
    }
    void log_message(const MessageData& m) override {
        if (m.m_severity & assertType::is_warn) return; // MESSAGE() and WARN() aren't failures
        emit(Obj()
                 .str("ev", "message")
                 .str("file", m.m_file)
                 .num("line", m.m_line)
                 .str("text", m.m_string)
                 .raw("subcases", array(subcases))
                 .raw("info", array(info())));
    }
    void test_case_skipped(const TestCaseData&) override {}
};
}  // namespace sysroad_report

REGISTER_REPORTER("sysroad", 1, sysroad_report::Reporter);
