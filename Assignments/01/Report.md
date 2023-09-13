# Results for Assignment 01

## Improvement Iterations

Here's a table showing the improvements I did to make the application go faster.  The **Time** column in the table represents the _wall-clock time_ for a program run.

| Version | Time | Speedup | Memory (KB) | Changes |
| :-----: | ---- | :-----: | :------: | ------- |
| [01](01.cpp) | 9.51s | &mdash; | 1041336 | Initial version - no changes. |
| [02](01.cpp) | 1.93s | 2.05x | 1041332 | Compiled with -Ofast to see about maximizing speed at (expected) cost to memory &mdash; apparently there was no memory tradeoff here. |
| [03](01.cpp) | 2.19s | .99x| 1041336 | Compiled with -O3 for aggressive, imprecise optimization for speed. |

## Profiling Analysis

### Initial Review

Looking at [01's profile](01.prof), the hottest function was `Transform::float4::perspectiveDivide() const`, which consumed around 28% of the program's execution time.  There's not a lot in that function, but it does three floating-point divisions, so perhaps that's something to try optimizing.

### Trying to make `perspectiveDivide()` go faster

`perspectiveDivide()` does several divisions by the same value `w`.  A common trick is instead of dividing by the same value multiple times, to compute the value's reciprocal, and multiple by that value.  This assumes that multiplication is a faster operation than division.
