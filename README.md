# Intro
Laph is a program designed for use in physics labs/experiments, born out of the
authors abhorrence of Excel. Instead of tables and cells, Laph uses nodes to
build a DAG (Directed Acyclic Graph) of computations.

Laph includes an embedded [Julia](https://julialang.org/) runtime, which is used
to compute the graph. Each node (except for input nodes) is backed by a Julia
file containing functions that map to each output socket; and this provides an
enormous amount of flexibility when it comes to processing the data.

## Requirements
* A C++14 compliant compiler (the `Makefile` defaults to g++)
* Qt 5.9 or higher
* Julia (any recent version should work, we recommend 0.6.0 or higher)

## Quickstart
Assuming we are in the cloned repo directory:

```bash
qmake-qt5
make
```
