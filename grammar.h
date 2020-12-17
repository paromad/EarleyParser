#pragma once
#include <unordered_set>


#include "symbol.h"

class Grammar {
protected:
    std::unordered_set<Symbol> symbols_;
    std::unordered_map<Symbol, std::unordered_set<std::vector<Symbol>>> rules_;
    Symbol start_;

public:
    explicit Grammar(Symbol start = Symbol(false)) : start_(start){};
    void AddRule(const Symbol &from, const std::vector<Symbol> &to) {
        rules_[from].insert(to);
        symbols_.insert(from);
        symbols_.insert(to.cbegin(), to.cend());
    }

    std::unordered_set<std::vector<Symbol>> &GetRightSides(const Symbol &s) {
        return rules_[s];
    }

    const std::unordered_set<std::vector<Symbol>> &GetRightSides(const Symbol &s) const {
        return rules_.at(s);
    }

    Symbol GetStart() const {
        return start_;
    }

    void SetStart(Symbol new_start) {
        start_ = new_start;
    }
};
