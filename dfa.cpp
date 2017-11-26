#include "dfa.h"

bool State::operator<(const State &rhs) const {
    return label < rhs.label;
}

bool State::operator==(const State &rhs) const {
    return label == rhs.label && output == rhs.output;
}

// need this since we can't just copy the pointer addresses - have to recalculate
DFA::DFA(const DFA& other) {
    states = other.states;
    alphabet = other.alphabet;
    transitions = std::vector<State*>(other.transitions.size());

    for(int i = 0; i < states.size(); ++i) {
        for(int j = 0; j < alphabet.size(); ++j) {
            int destIndex = std::distance(other.states.begin(), 
                    std::find(other.states.begin(), other.states.end(), 
                    *other.transitions.at(i * alphabet.size() + j)));

            transitions.at(i * alphabet.size() + j) = &states.at(destIndex);
        }
    }

    int startIndex = std::distance(other.states.begin(), std::find(
            other.states.begin(), other.states.end(), *other.start));

    start = &states.at(startIndex);
}

// does not verify that the file is correctly formatted
bool DFA::fromFile(std::string filename) {
    std::ifstream file(filename.c_str());

    if(!file.is_open())
        return false;

    std::string line;
    std::stringstream ss;
    int n = 0;

    // read alphabet
    std::getline(file, line);
    for(int i = 0; i < line.size(); ++i)
        alphabet.push_back(line[i]);

    // number of states
    std::getline(file, line);
    ss = std::stringstream(line);
    ss >> n;

    for(int i = 0; i < n; ++i) {
        std::getline(file, line);
        ss = std::stringstream(line);

        std::string label;
        bool output;

        ss >> label >> output;

        states.push_back({label, output});
    }

    // start state
    std::getline(file, line);
    start = &*(std::find_if(states.begin(), states.end(), 
            [&line](const State &s){return s.label == line;}));

    transitions.resize(n * alphabet.size());

    for(int i = 0; i < n * alphabet.size(); ++i) {
        std::getline(file, line);
        ss = std::stringstream(line);

        std::string sourceLabel, destinationLabel;
        char c;

        ss >> sourceLabel >> c >> destinationLabel;

        State &destination = *(std::find_if(states.begin(), states.end(), 
                [&destinationLabel](const State &s){return s.label == destinationLabel;}));

        int sourceIndex = std::distance(states.begin(), std::find_if(states.begin(), states.end(),
                [&sourceLabel](const State &s){return s.label == sourceLabel;}));

        int charIndex = std::distance(alphabet.begin(), std::find(alphabet.begin(), alphabet.end(), c));

        transitions.at(sourceIndex * alphabet.size() + charIndex) = &destination;
    }

    file.close();

    return true;
}

void DFA::saveToFile(std::string filename) const {
    std::ofstream file(filename);

    for(auto &c : alphabet)
        file << c;
    file << "\n";

    file << states.size() << "\n";

    for(auto &s : states) {
        file << s.label << " " << s.output << "\n";
    }

    file << start->label << "\n";

    for(int i = 0; i < states.size(); ++i) {
        for(int j = 0; j < alphabet.size(); ++j) {
            file << states.at(i).label << " " << alphabet.at(j) << " " << transitions.at(i * alphabet.size() + j)->label << "\n";
        }
    }

    file.close();
}

State &DFA::delta(const State &s, char c) {
    int stateIndex = std::distance(states.begin(), std::find(states.begin(), states.end(), s));
    int charIndex = std::distance(alphabet.begin(), std::find(alphabet.begin(), alphabet.end(), c));

    if(!(stateIndex < states.size() && charIndex < alphabet.size())) {
        std::cerr << "delta(" << s.label << ", " << c << ") is not defined!" << std::endl;
    }

    return *transitions.at(stateIndex * alphabet.size() + charIndex);
}

const State &DFA::delta(const State &s, char c) const {
    int stateIndex = std::distance(states.begin(), std::find(states.begin(), states.end(), s));
    int charIndex = std::distance(alphabet.begin(), std::find(alphabet.begin(), alphabet.end(), c));

    if(!(stateIndex < states.size() && charIndex < alphabet.size())) {
        std::cerr << "delta(" << s.label << ", " << c << ") is not defined!" << std::endl;
    }

    return *transitions.at(stateIndex * alphabet.size() + charIndex);
}

bool DFA::run(std::string s) const {
    const State *current = start;
    while(!s.empty()) {
        current = &delta(*current, s[0]);
        s = s.substr(1, std::string::npos);
    }

    return current->output;
}

void DFA::print() const {
    std::cout << "alphabet:" << std::endl << "\t";
    for(auto &c : alphabet) {
        std::cout << c << " ";
    }
    std::cout << std::endl;

    std::cout << "states:" << std::endl;
    for(auto &s : states) {
        std::cout << "\t" << (&s == start ? "> " : "  ") << s.label << ": " << (s.output ? "true" : "false") << std::endl;
    }

    std::cout << "transitions:" << std::endl;
    for(int i = 0; i < states.size(); ++i) {
        for(int j = 0; j < alphabet.size(); ++j) {
            std::cout << "\t" << "delta(" << states.at(i).label << ", " << alphabet.at(j)
                      << ") = " << transitions.at(i * alphabet.size() + j)->label << std::endl;
        }
    }
}

std::set<State> DFA::_getReachable() const {
    std::set<State> reachable {*start};
    std::set<State> currentStates {*start};

    while(!currentStates.empty()) {
        std::set<State> temp;

        for(auto &s : currentStates) {
            for(auto &c : alphabet) {
                temp.insert(delta(s, c));
            }
        }

        currentStates = setDifference(temp, reachable);

        for(auto &s : currentStates)
            reachable.insert(s);
    }

    return reachable;
}

// partitions the given states into the equivalence classes given by Myhill-Nerode
std::vector<std::set<State>> DFA::_getPartitioning(const std::set<State> &_states) const {
    std::set<State> accepting;
    std::set<State> denying;

    // partition into accepting/denying states
    for(auto s : _states) {
        if(s.output)
            accepting.insert(s);
        else
            denying.insert(s);
    }

    std::set<std::set<State>> partition {accepting, denying};
    std::set<std::set<State>> current {accepting};

    while(!current.empty()) {
        std::set<State> A = *current.begin();
        current.erase(current.begin());

        for(auto &c : alphabet) {
            std::set<State> X;
            for(auto &s : _states) {
                if(A.find(delta(s, c)) != A.end())
                    X.insert(s);
            }

            std::set<std::set<State>> removed;
            for(auto &Y : partition) {
                if(removed.find(Y) == removed.end()) {
                    std::set<State> intersection;
                    std::set<State> difference = setDifference(Y, X);

                    std::set_intersection(X.begin(), X.end(), Y.begin(), Y.end(),
                            std::inserter(intersection, intersection.begin()));

                    if(!intersection.empty() && !difference.empty()) {
                        partition.insert(intersection);
                        partition.insert(difference);

                        auto it = current.find(Y);
                        if(it != current.end()) {
                            current.insert(intersection);
                            current.insert(difference);
                            current.erase(it);
                        } else {
                            if(intersection.size() <= difference.size())
                                current.insert(intersection);
                            else
                                current.insert(difference);
                        }

                        removed.insert(Y);
                    }
                }
            }

            partition = setDifference(partition, removed);
        }
    }

    std::vector<std::set<State>> partitionVector;
    for(auto &Y : partition) {
        if(!Y.empty())
            partitionVector.push_back(Y);
    }

    return partitionVector;
}

// Hopcroft's algorithm, transcribed from Wikipedia article on DFA minimization
DFA DFA::minimize(bool simpleLabels) const {
    std::set<State> reachable = _getReachable();
    std::vector<std::set<State>> partition = _getPartitioning(reachable);

    DFA min;
    min.alphabet = alphabet;

    min.states.resize(partition.size());

    int n = 0;

    for(int i = 0; i < partition.size(); ++i) {
        auto &X = partition.at(i);

        std::string label;
        bool output;
        bool isStart = false;

        for(auto &s : X) {
            if(!simpleLabels)
                label += s.label;  // is this really the best way
            output = s.output;     // should all be the same
            isStart = isStart || s == *start;
        }

        if(simpleLabels)
            label = std::to_string(n);

        min.states.at(i) = {label, output};

        if(isStart)
            min.start = &min.states.at(i);

        ++n;
    }

    min.transitions.resize(min.states.size() * min.alphabet.size());

    for(auto it = partition.begin(); it != partition.end(); ++it) {
        auto &X = *it;

        for(int i = 0; i < alphabet.size(); ++i) {
            char c = alphabet.at(i);

            const State &dest = delta(*X.begin(), c);

            int srcIndex = std::distance(partition.begin(), it);
            int destIndex = std::distance(partition.begin(), std::find_if(partition.begin(), partition.end(),
                    [&dest](const std::set<State> &Y) {
                        return Y.find(dest) != Y.end();
                    }));

            min.transitions.at(srcIndex * min.alphabet.size() + i) = &min.states.at(destIndex);
        }
    }

    return min;
}

DFA DFA::complement() const {
    DFA comp = *this;

    for(auto &s : comp.states) {
        s.output = !s.output;
    }

    return comp;
}

bool DFA::sublanguageOf(const DFA &other) const {
    DFA intersection = fromProduct(*this, other.complement(), false);

    for(auto &s : intersection._getReachable())
        if(s.output)
            return false;

    return true;
}

bool DFA::equivalentTo(const DFA &other) const {
    // need both DFAs to be defined at least over the same alphabet
    // (technically they could recognize the same set of strings, but
    // I'm not sure if I want to deal with this...)
    if(this->alphabet.size() != other.alphabet.size())
        return false;
    else if(!std::is_permutation(this->alphabet.begin(), this->alphabet.end(), other.alphabet.begin()))
        return false;

    return this->sublanguageOf(other) && other.sublanguageOf(*this);
}

// assumes a and b have the same alphabet, should I throw an exception otherwise?
DFA DFA::fromProduct(const DFA &a, const DFA &b, bool isUnion) {
    DFA result;
    std::vector<std::tuple<State, State>> stateTuples;

    auto getLabel = [](const State &x, const State &y) {
        return "(" + x.label + "," + y.label + ")";
    };

    result.alphabet = a.alphabet; // = b.alphabet (we require them to all have the same alphabet)

    result.states.resize(a.states.size() * b.states.size());

    for(int i = 0; i < a.states.size(); ++i) {
        const State &p = a.states.at(i);

        for(int j = 0; j < b.states.size(); ++j) {
            const State &q = b.states.at(j);

            bool output = isUnion ? (p.output || q.output) : (p.output && q.output);

            stateTuples.push_back(std::make_tuple(p,q));
            result.states.at(i * b.states.size() + j) = {getLabel(p,q), output};

            if(p.label == a.start->label && q.label == b.start->label) {
                result.start = &result.states.at(i * b.states.size() + j);
            }
        }
    }

    result.transitions.resize(result.states.size() * result.alphabet.size());

    for(int i = 0; i < stateTuples.size(); ++i) {
        auto &s = stateTuples.at(i);

        for(int j = 0; j < result.alphabet.size(); ++j) {
            char c = result.alphabet.at(j);

            const State &x = a.delta(std::get<0>(s), c);
            const State &y = b.delta(std::get<1>(s), c);

            State &destination = *(std::find_if(result.states.begin(), result.states.end(),
                    [&x,&y,&getLabel](const State &state) {
                        return state.label == getLabel(x,y);
                    }));

            // each stateTuple has same index as corresponding state
            result.transitions.at(i * result.alphabet.size() + j) = &destination;
        }
    }

    return result;
}

template<class T> std::set<T> setDifference(const std::set<T> &a, const std::set<T> &b)   {
    std::set<T> out;

    std::set_difference(a.begin(), a.end(), b.begin(), b.end(), std::inserter(out, out.begin()));

    return out;
}

template std::set<State> setDifference(const std::set<State> &a, const std::set<State> &b);
template std::set<std::set<State>> setDifference(const std::set<std::set<State>> &a, const std::set<std::set<State>> &b);

