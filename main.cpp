#include "dfa.h"

int main(int argc, char **argv) {
    DFA a;
    DFA b;

    if(!(a.fromFile("dfas/two_ones.txt") && b.fromFile("dfas/most_two_zeros.txt"))) {
        std::cerr << "wtf" << std::endl;

        return -1;
    }

    a.print();
    b.print();

    DFA both = DFA::fromProduct(a, b, false);

    both.print();

    both.saveToFile("dfas/both.txt");

    DFA min = both.minimize(true);

    min.print();

    std::string tests[] = {
        "1101", "1111100111", "010100",
        "1010", "0111111110", "1000001",
        "0", "", "1", "101", "11"
    };

    min.saveToFile("dfas/min.txt");

    for(auto &s : tests) {
        std::cout << s << ": " << (min.run(s) ? "true" : "false")  << std::endl;
    }

    return 0;
}