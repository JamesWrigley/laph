# Intro
Laph is a program designed for use in physics labs/experiments, born out of the
authors abhorrence of Excel. Instead of tables and cells, Laph uses nodes to
build a DAG (Directed Acyclic Graph) of computations.

Laph includes an embedded [Julia](https://julialang.org/) runtime, which is used
to compute the graph. Each node (except for input nodes) is backed by a Julia
file containing functions that map to each output socket; and this provides an
enormous amount of flexibility when it comes to processing the data.

![Laph](laph.png)

## Requirements
* A C++14 compliant compiler *(technically C++17 right now, but it'll be back to C++14 soon)*
* Qt 5.11 or higher
* Julia 1.0 or higher

## Quickstart
Assuming we are in the cloned repo directory:

```bash
qmake-qt5
make
```
