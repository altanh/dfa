#ifndef DFA_H
#define DFA_H

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <tuple>
#include <set>
#include <map>

struct State {
    std::string label;
    bool output;

    bool operator<(const State &rhs) const;
    bool operator==(const State &rhs) const;
};

struct DFA {
    std::vector<State> states;
    std::vector<char> alphabet;
    std::map<std::tuple<std::string, char>, State> transitions;
    State start;

    DFA() {}

    bool fromFile(std::string filename);
    void saveToFile(std::string filename);

    bool run(std::string s);

    void print();

    DFA minimize(bool simpleLabels);

    static DFA fromProduct(DFA &a, DFA &b, bool isUnion);

private:
    std::set<State> _getReachable() const;
    std::vector<std::set<State>> _getPartitioning(const std::set<State> &_states) const;
};

template<class T> std::set<T> setDifference(const std::set<T> &a, const std::set<T> &b);

#endif