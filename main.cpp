#include <iostream>
#include <iomanip>

#include "dfa.h"

void usage(const char *prog) {
  std::cerr << "usage: " << prog << " dfa string" << std::endl;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    usage(argv[0]);
    return -1;
  }

  DFA dfa;

  if (!dfa.fromFile(std::string(argv[1]))) {
    std::cerr << "could not open " << argv[1] << std::endl;
    return -1;
  }

  std::cout << std::boolalpha << dfa.run(std::string(argv[2])) << std::endl;

  return 0;
}