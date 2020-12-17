#include <gtest/gtest.h>

#include <iostream>
#include <vector>

#include "earley_parser.h"
#include "grammar.h"
#include "solver.h"
#include "symbol.h"

std::pair<Symbol, std::vector<Symbol>> get_unary_rule() {
    Symbol S(false);
    std::vector<Symbol> to;
    Symbol a(true);
    to.emplace_back(a);

    return {S, to};
}

std::pair<Symbol, std::vector<Symbol>> get_binary_rule() {
    Symbol S(false);
    std::vector<Symbol> to;
    Symbol a(true);
    Symbol b(false);
    to.emplace_back(a);
    to.emplace_back(b);

    return {S, to};
}

Situation get_binary_situation() {
    auto rule = get_binary_rule();
    return {rule.first, rule.second};
}


class PSP_grammar {
public:
    PSP_grammar() : start_(false), open_(true), close_(true), g_(start_) {
        g_.AddRule(start_, {});
        g_.AddRule(start_, {open_, start_, close_, start_});
    }

    const Grammar &GetGrammar() const {
        return g_;
    }

    Symbol GetOpen() const {
        return open_;
    }

    Symbol GetStart() const {
        return start_;
    }

    Symbol GetClose() const {
        return close_;
    }

private:
    Symbol start_;
    Symbol open_;
    Symbol close_;
    Grammar g_;
};

TEST(Symbols, Comparations) {
    ASSERT_FALSE(Symbol(false) == Symbol(false));
    Symbol s1(false);
    Symbol s2 = s1;
    ASSERT_TRUE(s1 == s2);
}

TEST(Grammar, Basic) {
    auto rule = get_unary_rule();
    Symbol S = rule.first;
    std::vector<Symbol> to = rule.second;

    Grammar g(S);
    ASSERT_TRUE(g.GetStart() == S);

    g.AddRule(S, to);
    ASSERT_EQ(g.GetRightSides(S).size(), 1);
    ASSERT_TRUE(*g.GetRightSides(S).cbegin() == to);

    Symbol S1(false);
    g.SetStart(S1);
    ASSERT_TRUE(g.GetStart() == S1);
}

TEST(Situation, Basic) {
    auto rule = get_unary_rule();
    Symbol S = rule.first;
    std::vector<Symbol> to = rule.second;
    Grammar g(S);
    g.AddRule(S, to);

    Situation s(S, to, 3);
    ASSERT_EQ(s.GetStartPosition(), 3);
    ASSERT_TRUE(s.GetSymbolFrom() == S);
    ASSERT_FALSE(s.IsEnded(g));
    ASSERT_FALSE(s.IsDotInEnd());
    ASSERT_TRUE(s.GetSymbolAfterDot() == to[0]);
}

TEST(Situation, BasicUnary) {
    auto rule = get_unary_rule();
    Symbol S = rule.first;
    std::vector<Symbol> to = rule.second;
    Grammar g(S);
    g.AddRule(S, to);

    Situation s(S, to);
    Situation new_s = s.GetNextSituation();
    ASSERT_EQ(new_s.GetStartPosition(), 0);
    ASSERT_TRUE(new_s.GetSymbolFrom() == S);
    ASSERT_TRUE(new_s.IsEnded(g));
    ASSERT_TRUE(new_s.IsDotInEnd());
    ASSERT_THROW(new_s.GetSymbolAfterDot(), std::logic_error);

    ASSERT_FALSE(s == new_s);
}

TEST(Situation, BasicBinary) {
    auto rule = get_binary_rule();
    Symbol S = rule.first;
    std::vector<Symbol> to = rule.second;
    Grammar g(S);
    g.AddRule(S, to);

    Situation s(S, to);
    s = s.GetNextSituation();
    ASSERT_EQ(s.GetStartPosition(), 0);
    ASSERT_TRUE(s.GetSymbolFrom() == S);
    ASSERT_FALSE(s.IsEnded(g));
    ASSERT_FALSE(s.IsDotInEnd());
    ASSERT_TRUE(s.GetSymbolAfterDot() == to[1]);
}

TEST(SituationList, basic) {
    Situation s = get_binary_situation();
    SituationList list;

    ASSERT_TRUE(s.GetSymbolAfterDot().IsTerminal());
    list.AddSituation(s);
    ASSERT_EQ(list.dot_before_terminal_.size(), 1);
    ASSERT_EQ(list.size(), 1);

    s = s.GetNextSituation();
    ASSERT_FALSE(s.GetSymbolAfterDot().IsTerminal());
    list.AddSituation(s);
    ASSERT_EQ(list.dot_before_nonterminal_.size(), 1);
    ASSERT_EQ(list.size(), 2);

    s = s.GetNextSituation();
    list.AddSituation(s);
    ASSERT_EQ(list.dot_in_end_.size(), 1);
    ASSERT_EQ(list.size(), 3);

    list.AddSituation(s);
    ASSERT_EQ(list.size(), 3);
}

TEST(EarlyParser, PSP) {
    PSP_grammar psp;

    EarleyParser parser(psp.GetGrammar());
    const Grammar &earley_grammar = parser.GetGrammar();

    SituationList list;
    Situation start_situation = Situation(earley_grammar.GetStart(), {psp.GetStart()});
    list.AddSituation(start_situation);

    parser.Predict(list, 0);
    ASSERT_EQ(list.size(), 3);
    Situation open_S_close_S_situation(psp.GetStart(), {psp.GetOpen(), psp.GetStart(), psp.GetClose(), psp.GetStart()});
    ASSERT_EQ(list.dot_before_terminal_.count(open_S_close_S_situation), 1);
    ASSERT_EQ(list.dot_in_end_.count(Situation(psp.GetStart(), {})), 1);

    std::vector<SituationList> parsing_list{list};

    parser.Complete(parsing_list, 0);
    ASSERT_EQ(parsing_list[0].dot_in_end_.count(start_situation.GetNextSituation()), 1);

    parsing_list.emplace_back(SituationList());

    size_t size = parsing_list[1].size();
    parser.Scan(parsing_list[0], parsing_list[1], psp.GetClose());
    ASSERT_EQ(size, parsing_list[1].size());

    parser.Scan(parsing_list[0], parsing_list[1], psp.GetOpen());
    ASSERT_EQ(parsing_list[1].dot_before_nonterminal_.count(open_S_close_S_situation.GetNextSituation()), 1);
}

TEST(Solution, PSP) {
    std::istringstream grammar_in("2 S\n S\n aSbS\n S -\n");
    Solver solver(grammar_in, std::cout);

    ASSERT_TRUE(solver.Solve("-"));
    ASSERT_TRUE(solver.Solve("ab"));
    ASSERT_TRUE(solver.Solve("abab"));
    ASSERT_TRUE(solver.Solve("aabb"));
    ASSERT_TRUE(solver.Solve("aabbabaababb"));

    ASSERT_FALSE(solver.Solve("ba"));
    ASSERT_FALSE(solver.Solve("bbaa"));
    ASSERT_FALSE(solver.Solve("abba"));
    ASSERT_FALSE(solver.Solve("aaba"));
    ASSERT_FALSE(solver.Solve("babbabababbb"));
    ASSERT_FALSE(solver.Solve("ababababababababaaa"));
    ASSERT_FALSE(solver.Solve("cccc"));
    ASSERT_FALSE(solver.Solve("abc"));
}
