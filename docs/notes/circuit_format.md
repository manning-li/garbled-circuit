# Circuit Format

This project uses [Bristol Fashion MPC
Circuits](https://nigelsmart.github.io/MPC-Circuits/).

### Example: `and.txt`

``` text
1 3
2 1 1
1 1
2 1 0 1 2 AND
```

-   `1 3`: 1 gate and 3 wires.
-   `2 1 1`: 2 input parties, each containing 1 wire.
-   `1 1`: 1 output party, containing 1 wire.
-   `2 1 0 1 2 AND`: number of input wires (2), number of output wires (1), indices of input wires (0 1), indices of output wires (2), type of gate (AND).

The circuit reresents ` wire[0] AND wire[1] = wire[2]`.

### Example: `eq.txt`

```text
1 2
0
1 1
1 1 0 1 EQ
```
-   `1 1 0 1 EQ`: number of input wires (1), number of output wires (1), the value to assign (0), indices of output wires (1), type of gate (EQ).

The circuit reresents ` wire[1] = 0`