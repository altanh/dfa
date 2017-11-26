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

/*
    transitions[j * n + i] = delta(states[j], alphabet[i]),
    where n = number of states
 */

struct DFA {
    std::vector<State> states;
    std::vector<char> alphabet;
    //std::map<std::tuple<std::string, char>, State> transitions;
    std::vector<State*> transitions;
    State *start;

    DFA() {}
    DFA(const DFA& other);

    bool fromFile(std::string filename);
    void saveToFile(std::string filename) const;

    State &delta(const State &s, char c);

    // read-only
    const State &delta(const State &s, char c) const;

    bool run(std::string s) const;

    void print() const;

    DFA minimize(bool simpleLabels) const;
    DFA complement() const;

    bool sublanguageOf(const DFA &other) const;
    bool equivalentTo(const DFA &other) const;

    static DFA fromProduct(const DFA &a, const DFA &b, bool isUnion);

private:
    
    std::set<State> _getReachable() const;
    std::vector<std::set<State>> _getPartitioning(const std::set<State> &_states) const;
};

template<class T> std::set<T> setDifference(const std::set<T> &a, const std::set<T> &b);

#endif