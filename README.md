Filter lines present multiple times.

# Usage

```
huniq [FILE]...
```

# Example

```sh
huniq <<<'first
second
second
third
fourth
third
third
fourth
first'
```

ouput:
```
first
second
third
fourth
```

# Compilation

Just run [`xmake`](https://github.com/xmake-io/xmake).

*Or `g++ -std=c++17 -O3 -I include huniq.cpp -o huniq`*

# Installation

```sh
xmake build huniq && xmake install huniq
```
