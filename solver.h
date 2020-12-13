#pragma once

#include <iostream>

#include "earley_parser.h"


class Solver {
    std::unordered_map<char, Symbol> mapping;
    Grammar grammar;
public:
    Solver(std::istream& cin = std::cin, std::ostream& cout = std::cout) {
        for (char c = 'a'; c <= 'z'; ++c) {
            mapping.insert({c, Symbol(true)});
        }
        for (char c = 'A'; c < 'Z'; ++c) {
            mapping.insert({c, Symbol(false)});
        }

        int n;
        char StartNonTerm;
        cout << "Rules count, StartNonTerm" << std::endl;
        cin >> n >> StartNonTerm;

        grammar.SetStart(mapping.at(StartNonTerm));

        for (int i = 0; i < n; ++i) {
            cout << "rule No " << i + 1 << " Letter From, String To('-' for eps)" << std::endl;
            char from;
            std::string to;
            cin >> from >> to;
            std::vector<Symbol> symbols_to;
            if (to != "-") {
                for (char c : to) {
                    symbols_to.push_back(mapping.at(c));
                }
            }
            Symbol symbol_from = mapping.at(from);
            grammar.AddRule(symbol_from, symbols_to);
        }
    }

    bool Solve(const std::string& word) const {
        EarleyParser parser(grammar);
        std::vector<Symbol> symbol_word;
        if (word != "-") {
            for (char c : word) {
                symbol_word.push_back(mapping.at(c));
            }
        }
        return parser.Accept(symbol_word);
    }
};