# Homeworlds AI

An AI for Homeworlds that uses Negamax.

## Installation

```
curl https://raw.githubusercontent.com/philsquared/Catch/master/single_include/catch.hpp > tests/catch.hpp
make
```

## Game Description Format

Will make a grammar or something once format is finalized. For now, here's an example. The first line is "[number of players] [number of homeworlds built] [current player]".

```
2 2 1
Alice (1, b1y2) 1g3 1g1 1g2
Bob (2, g3y2) 2b3 2b1 2b2
Sirius (r3) 1g1
Pluto (g1) 2b1
```

## Usage

`./main` takes the current game state as input and outputs the AI's moves for the turn.
`.judge` takes a game state and a turn as input and outputs the new game state, as well as whether a player has won the game.
`python run_game.py [initial game state file]` runs the AI against itself using the judge.
