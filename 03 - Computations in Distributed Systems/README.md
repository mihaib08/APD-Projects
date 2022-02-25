# APD - Computations in Distributed Systems

Barbu Mihai-Eugen, 335CA [2021-2022]

## Overview

The aim of this project is to __distribute__ the calculus in a _cluster_-like system, where each __coordinator__ has a number of __workers__ assigned to it, each of the workers having to compute a _chunk_ of an array. Therefore, the calculus need to be distributed as balanced as possible between the workers, in order to ensure the __scalability__ of the system.

----

## Implementation

All sources are in _sol/_:

- _tema3.cpp_ - each process has the variables:
    - `rank` - rank of the process
    - `numTasks` - no. processes
    - `(std::vector<int *>) clusters` - data of each cluster
        - each element contains the information of a cluster

- _utils.cpp_
    - `getTopology()` - displays the topology using the data from ``clusters``
    - `displayMessage()` - message being displayed for each `MPI_Send()`

- _coordinator.cpp_ - defines the computations done by each coordinator, depending on the communication error (0/1):
    - `findTopology()`
    / `bonusTopology()` - finds the topology of the entire system

    - `rootCompute()`
    / `rootBonusCompute()` - divides the computations from the process w/ ```rank = 0``` to the other __connected__ processes

    - `coordinatorCompute()`
    / `coordinator{1,2}Compute`
            - divides the computations for the other coordinators (having the $rank \in \{1,2\}$)

## Operations

#### 1. Topology

Each __coordinator__ processes the data given in its input file, then it generates the cluster ```(int *) curr``` as follows:
- curr[0] - coordinator rank
- curr[1] - no. _workers_
- curr[2..] - workers' ranks

Each coordinator sends the cluster data to the other coordinators (0 -> 1,2; 1 -> 0,2; 2 -> 0,1).

When each **coordinator** finds the entire _topology_, it is sent to the corresponding **workers**.

#### 2. Computations

The first coordinator, i.e. w/ `rank = 0` (_rootCompute()_), _sends_ the number of elements of the array that has to be computed, generates the array, then:

1. computes the boundaries for each of its __workers__
2. divides the calculus for the other coordinators:

    - `0` sends to `1` its upper limit,
      `1` sends to `0` its upper limit,
      the latter being sent from `0` to `2`

3. divides the array depending on these boundaries and sends each chunk to the workers and coordinators as previously assigned
4. waits (`MPI_RECV()`) to receive the results from the other workers and coordinators

The other coordinators - `1`, `2` (_coordinatorCompute()_) - receive `N` = size of the array,
as well as the _lower_ limit assigned to them, then each:

1. computes the boundaries for each of its __workers__
2. sends the upper limit back to `0`
3. receives the subarray assigned to it and divides it for the __workers__
4. waits for the results from the workers and sends them to `0`

#### 3. Fault tolerance

When the `(0, 1)` link is not available, `0` will communicate with `1` _through_ `2`.

Therefore, the communication `0 <-> 1` becomes `0 <-> 2 <-> 1`.

This way, the topology will be found as follows:

- `0` and `1` send their data to `2`, then `2` sends the data regarding:
    - clusters `0` and `2` -> `1`
    - clusters `1` and `2` -> `0`

- each coordinator sends the topology to the workers assigned to it

As for the computation of the array, `0` sends `v[curr..]` to `2`, where `curr` - upper boundary assigned to the workers of `0`, then `2` divides the received array between itself and `1`.

----

### Obs.

- each __worker__ initially makes a call to `MPI_RECV()` using the parameter __MPI\_ANY\_SOURCE__, then it extracts the coordinator rank from the __status__ field

- multiple tags are used for differentiating the content of the variables being sent across the system

- the __workers__ always have the same implementation, as they __only__ communicate with their coordinator
