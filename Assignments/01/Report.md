# Results for Assignment 01

## Improvement Iterations

Here's a table showing the improvements I did to make the application go faster.  The **Time** column in the table represents the _wall-clock time_ for a program run.

| Version | Time | Speedup | Memory (KB) | Changes |
| :-----: | ---- | :-----: | :------: | ------- |
| [01](01.cpp) | 9.51s | &mdash; | 1041336 | Initial version - no changes. |
| 02 | 1.93s | 4.93x | 1041332 | Compiled with -Ofast to see about maximizing speed at (expected) cost to memory. |
| 03 | 2.19s | 4.34x| 1041336 | Compiled with -O3 for aggressive, imprecise optimization for speed. |
| 04 | 3.86s | 2.46x| 1041336 | Compiled with -Os for optimization of speed **and** memory |

## Speedup Analysis and Review

Using the optimization flags was an effective tool for significant speedup across each time trial, but the fastest improvement was using the -Ofast compilation flag. Interestingly, this optimization flag which is known for spending large spaces of memory in exchange for speedier performance actually used the least memory of all 4 trials, if by a marginal quanitity.

### Takeaways

With an impressive speedup greater than 4x, the -Ofast and -O3 compilation flags were effective and, in observed tests, accurate. The speedup of the -Os optimization flag was effective as well, but it did not seem to reduce the memory costs of running this process at all, reducing the usefulness of compiling with this option.

It seems that the requirements of the 01.cpp program result in little room for change to the memory configuration of the process data. As a result, employing "memory-hungry" optimization techniques may be effective.
