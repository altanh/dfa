#include "dfa.h"

bool State::operator<(const State &rhs) const {
    return label < rhs.label;
}

bool State::operator==(const State &rhs) const {
    return label == rhs.label && output == rhs.output;
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
    start = *(std::find_if(states.begin(), states.end(), 
            [&line](const State &s){return s.label == line;}));

    for(int i = 0; i < n * alphabet.size(); ++i) {
        std::getline(file, line);
        ss = std::stringstream(line);

        std::string sourceLabel, destinationLabel;
        char c;

        ss >> sourceLabel >> c >> destinationLabel;

        State destination = *(std::find_if(states.begin(), states.end(), 
                [&destinationLabel](const State &s){return s.label == destinationLabel;}));

        transitions.insert(std::pair<std::tuple<std::string, char>, State>(
                std::make_tuple(sourceLabel, c), destination));
    }

    file.close();

    return true;
}

void DFA::saveToFile(std::string filename) {
    std::ofstream file(filename);

    for(auto &c : alphabet)
        file << c;
    file << "\n";

    file << states.size() << "\n";

    for(auto &s : states) {
        file << s.label << " " << s.output << "\n";
    }

    file << start.label << "\n";

    for(auto &t : transitions) {
        file << std::get<0>(t.first) << " " << std::get<1>(t.first) << " " << t.second.label << "\n";
    }

    file.close();
}

bool DFA::run(std::string s) {
    State current = start;
    while(!s.empty()) {
        current = transitions.at(std::make_tuple(current.label, s[0]));
        s = s.substr(1, std::string::npos);
    }

    return current.output;
}

void DFA::print() {
    std::cout << "alphabet:" << std::endl << "\t";
    for(auto &c : alphabet) {
        std::cout << c << " ";
    }
    std::cout << std::endl;

    std::cout << "states:" << std::endl;
    for(auto &s : states) {
        std::cout << "\t" << (s.label == start.label ? "> " : "  ") << s.label << ": " << (s.output ? "true" : "false") << std::endl;
    }

    std::cout << "transitions:" << std::endl;
    for(auto &t : transitions) {
        std::cout << "\tdelta(" << std::get<0>(t.first) << ", " << std::get<1>(t.first) << ") = " << t.second.label << std::endl;
    }
}

std::set<State> DFA::_getReachable() const {
    std::set<State> reachable {start};
    std::set<State> currentStates {start};

    while(!currentStates.empty()) {
        std::set<State> temp;

        for(auto &s : currentStates) {
            for(auto &c : alphabet) {
                temp.insert(transitions.at(std::make_tuple(s.label, c)));
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
                if(A.find(transitions.at(std::make_tuple(s.label, c))) != A.end())
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

    return std::vector<std::set<State>>(partition.begin(), partition.end());
}

// Hopcroft's algorithm, transcribed from Wikipedia article on DFA minimization
DFA DFA::minimize(bool simpleLabels) {
    std::set<State> reachable = _getReachable();
    std::vector<std::set<State>> partition = _getPartitioning(reachable);

    DFA min;
    min.alphabet = alphabet;

    int n = 0;
    for(auto &X : partition) {
        std::string label;
        bool output;
        bool isStart = false;

        for(auto &s : X) {
            if(!simpleLabels)
                label += s.label;  // is this really the best way
            output = s.output;     // should all be the same
            isStart = isStart || s.label == start.label;
        }

        if(simpleLabels)
            label = std::to_string(n);

        min.states.push_back({label, output});

        if(isStart)
            min.start = min.states.back();

        ++n;
    }

    for(auto it = partition.begin(); it != partition.end(); ++it) {
        auto &X = *it;

        for(auto &c : alphabet) {
            std::string str = X.begin()->label;
            State &dest = transitions.at(std::make_tuple(str, c));

            int srcIndex = std::distance(partition.begin(), it);
            int destIndex = std::distance(partition.begin(), std::find_if(partition.begin(), partition.end(),
                    [&dest](const std::set<State> &Y) {
                        return Y.find(dest) != Y.end();
                    }));

            min.transitions.insert(std::pair<std::tuple<std::string, char>, State>(
                    std::make_tuple(min.states[srcIndex].label, c), 
                    min.states[destIndex]));
        }
    }

    return min;
}

// assumes a and b have the same alphabet, should I throw an exception otherwise?
DFA DFA::fromProduct(DFA &a, DFA &b, bool isUnion) {
    DFA result;
    std::vector<std::tuple<State, State>> stateTuples;

    auto getLabel = [](const State &x, const State &y) {
        return "(" + x.label + "," + y.label + ")";
    };

    result.alphabet = a.alphabet; // = b.alphabet (we require them to all have the same alphabet)

    for(auto &p : a.states) {
        for(auto &q : b.states) {
            bool output = isUnion ? (p.output || q.output) : (p.output && q.output);

            stateTuples.push_back(std::make_tuple(p,q));
            result.states.push_back({getLabel(p,q), output});

            if(p.label == a.start.label && q.label == b.start.label)
                result.start = result.states.back(); //the one we just added
        }
    }

    for(auto &s : stateTuples) {
        for(auto &c : result.alphabet) {
            State x = a.transitions.at(std::make_tuple(std::get<0>(s).label, c));
            State y = b.transitions.at(std::make_tuple(std::get<1>(s).label, c));

            State destination = *(std::find_if(result.states.begin(), result.states.end(),
                    [&x,&y,&getLabel](const State &state) {
                        return state.label == getLabel(x,y);
                    }));

            result.transitions.insert(std::pair<std::tuple<std::string, char>, State>(
                std::make_tuple(getLabel(std::get<0>(s), std::get<1>(s)), c), destination));
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

