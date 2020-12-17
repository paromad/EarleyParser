#pragma once

#include <functional>


class Symbol {
    int id_;
    bool is_terminal_;

    friend std::hash<Symbol>;

public:
    explicit Symbol(bool is_terminal) : is_terminal_(is_terminal) {
        static int cnt = 0;
        cnt++;
        id_ = cnt;
    }

    Symbol(const Symbol &s) = default;

    Symbol &operator=(const Symbol &s) = default;

    bool IsTerminal() const {
        return is_terminal_;
    }

    bool operator==(const Symbol &other) const {
        return id_ == other.id_;
    }

    bool operator!=(const Symbol &other) const {
        return !operator==(other);
    }
};


template<>
class std::hash<Symbol> {
public:
    size_t operator()(const Symbol &obj) const {
        return std::hash<int>()(obj.id_);
    }
};


template<class T>
void hash_combine(std::size_t &seed, const T &v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b7 + (seed << 6) + (seed >> 2);
}


template<>
class std::hash<std::vector<Symbol>> {
public:
    size_t operator()(const std::vector<Symbol> &obj) const {
        size_t seed = 0;
        for (const auto &v : obj) {
            hash_combine(seed, v);
        }
        return seed;
    }
};
