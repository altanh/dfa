# DFA

## DFA file structure
```
01             // alphabet as a string
2              // number of states
q_0 0          // state label and output (0 for false, 1 for true)
q_1 1
q_0            // start state
q_0 0 q_0      // state transitions, (src character dest)
q_0 1 q_1      // ordering is not important, so long no transitions are missing
q_1 0 q_1
q_1 1 q_1
```