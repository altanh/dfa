# TODO

## Restructure/redesign state transitions
Currently, the state transition function for each DFA is stored as a map from `(label, char) -> State`. This is questionable - since I'm storing the DFA's states internally in a vector (which has a built-in array-type ordering), I could simply store the transitions as a single vector containing `State&`s, with specific transitions given by an indexing scheme dependent on the internal ordering of the alphabet and states.

This would remove the map inefficiencies and also some unnecessary state-label conversions/lookups. 

*UPDATE*: I have now switched to the above method for storing state transitions. Internally, the transitions are stored in a vector of `State*`s (I realized that references must be initialized upon declaration, not to mention that they need to be wrapped in a class to even be stored in STL containers). This was a bit tricky to deal with, since every time a DFA is copy-constructed I need to recalculate the pointer addresses. In the end, I think this approach is probably faster, but I'm not sure if the added pointer complexity will hurt in the long term.