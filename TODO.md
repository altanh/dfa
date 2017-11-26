# TODO

## Restructure/redesign state transitions
Currently, the state transition function for each DFA is stored as a map from `(label, char) -> State`. This is questionable - since I'm storing the DFA's states internally in a vector (which has a built-in array-type ordering), I could simply store the transitions as a single vector containing `State&`s, with specific transitions given by an indexing scheme dependent on the internal ordering of the alphabet and states.

This would remove the map inefficiencies and also some unnecessary state-label conversions/lookups. 