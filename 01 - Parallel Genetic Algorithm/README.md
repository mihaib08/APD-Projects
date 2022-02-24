# APD - Parallel Genetic Algorithm

Barbu Mihai-Eugen, 335CA [2021]

----

## Overview

The aim of this project is to provide a parallel implementation of a _genetic_ algorithm used for solving the __knapsack__ problem (0-1).

The sequential algorithm is in ```skel/```. The __parallel__ solution is in ```sol/```.

## Implementation

Using the sources in ```skel/``` as starting point, the following structure is used for _dividing_ the computations on each __thread__:

- (struct) _th\_arg_ (```utils.h```) - used for the variables passed as arguments at the initialisation of the threads
    - id - thread index
    - *current_generation
    - *next_generation - generation which is processed from _current\_generation_
    - generations_count - no. iterations until the final result is reached
    - object_count / *objects
    - sack_capacity

    - P - no. threads
    - *barrier - **pointer** to the barrier used
                 for thread synchronization
    
    - *st, *en - arrays containing the upper and lower
                 bounds of the computations on each thread
                 

Therefore, the function _thread\_genetic()_ is used for processing each _generations\_count_ iterations, dividing the computations done by each thread:

- using the index of a thread, _id_, and the total no. threads _P_, the indices used for iteration in _*current\_generation_ and _*next\_generation_ are calculated
    - _start_, _end_ - depending on _object\_count_
    - _s_, _e_ - depending on _count_

- **barrier** is used for thread synchronization in the following situations:

    - initialization of the arrays (_current\_generation_, _next\_generation_)
    - _current\_generation_ parallel sort
    - calculus for getting a new generation

### Obs.

1)  For sorting, I started from the _qsort()_ function for each subarray corresponding to a thread; then the results are merged.

    This approach has the **time** complexity **O(N/P * log(N/P) + N * P)**, while the OETS solution would have _O(N * N/P)_.
