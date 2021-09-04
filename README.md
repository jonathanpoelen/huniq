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
