#include <iostream>
#include <unordered_map>

#include "solver.h"

int main() {
    Solver solver{};

    std::string s;
    while (std::cin >> s) {
        if (solver.Solve(s)) {
            std::cout << "Accept\n";
        } else {
            std::cout << "Discard\n";
        }
    }

    return 0;
}
