#pragma once

#include "symbol.h"
#include "grammar.h"

class Situation {
    Symbol from_;
    std::vector<Symbol> to_;
    size_t start_position_;
    size_t dot_position_;

public:
    Situation(Symbol from, const std::vector<Symbol> &to, size_t start_position = 0)
            : from_(from), to_(to), start_position_(start_position), dot_position_(0){};

    bool IsDotInEnd() const {
        return dot_position_ == to_.size();
    }

    Symbol GetSymbolAfterDot() const {
        if (!IsDotInEnd()) {
            return to_[dot_position_];
        }
        throw std::logic_error("dot in the end, can't get symbol after dot");
    }

    Situation GetNextSituation() const {
        Situation new_situation = *this;
        ++new_situation.dot_position_;
        return new_situation;
    }

    size_t GetStartPosition() const {
        return start_position_;
    }

    Symbol GetSymbolFrom() const {
        return from_;
    }

    bool IsEnded(const Grammar &grammar) const {
        Symbol from = grammar.GetStart();
        const std::vector<Symbol> &to = *grammar.GetRightSides(grammar.GetStart()).begin();
        return (IsDotInEnd()) && (from_ == from) && (to_ == to);
    }

    bool operator==(const Situation &other) const {
        return from_ == other.from_ && start_position_ == other.start_position_ &&
               dot_position_ == other.dot_position_ && to_ == other.to_;
    }

    friend std::hash<Situation>;
};

template<>
class std::hash<Situation> {
public:
    size_t operator()(const Situation &situation) const {
        size_t seed = 0;
        hash_combine(seed, situation.from_);
        hash_combine(seed, situation.to_);
        hash_combine(seed, situation.start_position_);
        hash_combine(seed, situation.dot_position_);
        return seed;
    }
};

struct SituationList {
    std::unordered_set<Situation> dot_before_terminal_;
    std::unordered_set<Situation> dot_before_nonterminal_;
    std::unordered_set<Situation> dot_in_end_;

    size_t size() const {
        return dot_in_end_.size() + dot_before_nonterminal_.size() + dot_before_terminal_.size();
    }

    void AddSituation(const Situation &situation) {
        if (situation.IsDotInEnd()) {
            dot_in_end_.insert(situation);
        } else if (situation.GetSymbolAfterDot().IsTerminal()) {
            dot_before_terminal_.insert(situation);
        } else {
            dot_before_nonterminal_.insert(situation);
        }
    };

    void AddSituations(const std::vector<Situation> &situations_to_add) {
        for (const auto &situation : situations_to_add) { AddSituation(situation); }
    }
};

class EarleyParser {
    Grammar grammar_;

public:
    explicit EarleyParser(const Grammar &grammar) : grammar_(grammar) {
        Symbol new_start(false);
        grammar_.AddRule(new_start, {grammar.GetStart()});
        grammar_.SetStart(new_start);
    };

    Symbol GetStart() const {
        return grammar_.GetStart();
    }

    const Grammar& GetGrammar() const {
        return grammar_;
    }

    bool Predict(SituationList &situation_list, size_t j) const {
        std::vector<Situation> situations_to_add;
        for (const auto &situation : situation_list.dot_before_nonterminal_) {
            Symbol from = situation.GetSymbolAfterDot();
            for (const auto &right_side : grammar_.GetRightSides(from)) {
                situations_to_add.emplace_back(from, right_side, j);
            }
        }
        size_t old_size = situation_list.size();
        situation_list.AddSituations(situations_to_add);
        return old_size != situation_list.size();
    }

    void Scan(const SituationList &situation_list, SituationList &next_situation_list, Symbol letter) const {
        for (const auto &situation : situation_list.dot_before_terminal_) {
            if (situation.GetSymbolAfterDot() == letter) {
                next_situation_list.AddSituation(situation.GetNextSituation());
            }
        }
    }

    bool Complete(std::vector<SituationList> &parsing_list, size_t j) const {
        std::vector<Situation> situations_to_add;
        for (const auto &inserting_situation : parsing_list[j].dot_in_end_) {
            Symbol from = inserting_situation.GetSymbolFrom();
            size_t start_position = inserting_situation.GetStartPosition();
            for (const auto &situation : parsing_list[start_position].dot_before_nonterminal_) {
                if (from != situation.GetSymbolAfterDot()) {
                    continue;
                }
                situations_to_add.emplace_back(situation.GetNextSituation());
            }
        }

        size_t old_size = parsing_list[j].size();
        parsing_list[j].AddSituations(situations_to_add);
        return old_size != parsing_list[j].size();
    }

    bool Accept(const std::vector<Symbol> &word) const {
        std::vector<SituationList> parsing_list(word.size() + 1);
        Symbol from = grammar_.GetStart();
        const std::vector<Symbol> &to = *grammar_.GetRightSides(grammar_.GetStart()).begin();
        parsing_list[0].AddSituation(Situation(from, to));

        bool is_changed = false;
        do {
            is_changed = Predict(parsing_list[0], 0);
            is_changed |= Complete(parsing_list, 0);
        } while (is_changed);

        for (size_t i = 0; i < word.size(); ++i) {
            Scan(parsing_list[i], parsing_list[i + 1], word[i]);
            do {
                is_changed = Predict(parsing_list[i + 1], i + 1);
                is_changed |= Complete(parsing_list, i + 1);
            } while (is_changed);
        }
        const auto &last_situation_list = parsing_list[word.size()].dot_in_end_;

        return std::any_of(last_situation_list.begin(), last_situation_list.end(),
                           [this](const Situation &situation) { return situation.IsEnded(grammar_); });
    }
};
