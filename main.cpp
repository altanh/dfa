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

    min.saveToFile("dfas/min.txt");

    std::string tests[] = {
        "1101", "1111100111", "010100",
        "1010", "0111111110", "1000001",
        "0", "", "1", "101", "11", "001", "0001", "00000"
    };

    for(auto &s : tests) {
        std::cout << "both(" << s << "): " << (both.run(s) ? "true" : "false")  << std::endl;
        std::cout << "min(" << s << "): " << (min.run(s) ? "true" : "false")  << std::endl;
    }

    std::cout << "both = min? " << (both.equivalentTo(min) ? "true" : "false") << std::endl;

    return 0;
}