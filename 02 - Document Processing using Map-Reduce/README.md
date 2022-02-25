# APD - Document Processing using the Map-Reduce Paradigm

Mihai-Eugen Barbu, 335CA [2021-2022]

----

## Structure
The following structure was used for implementing the processing of the documents:

  - `default` 
      - _Tema2_ - represents the coordinator thread
      - _MRThread_ - solves the Map, Reduce tasks assigned to a thread

  - `map` - implements the Map tasks

  - `reduce` - implements the Reduce tasks

  - `utils`
      - contains the _Rank_ class, which is used for computing the rank of a document using the Fibonacci numbers
        - each number in the Fibonacci sequence takes __logarithmic__ time to be computed

  Each of the `map`/`reduce` packets contains the classes:

  - _Task_
    - implements the method _solveTask()_

  - _Result_
    - aggregates the results from _Task_

  Additionally, `map` also contains the _Results_ class, which is used for combining the results of the Map stage for a specific document. Therefore, a __Map__ (document -> _MapResults_) is being used.

<!-- ---- -->

## Implementation

The tasks (Map/Reduce) are being initialised using the following fields:

  - docs - list of the _paths_ to the input files
  - docNames - list containing the names of the input files

Then, each task is being processed as follows:

  - Map (in _genDocumentTasks()_)

    - name of the document
    
    - starting offset corresponding to the task

    - size of the fragment to be processed
      - considering the case when a word would be broken in between 2 tasks, the folowing check is done:
        - if the last character of the current fragment, as well as the first one of the next fragment are __alphanumeric__ characters, the size of the current fragment is __increased__ in order to _entirely_ contain the _broken_ word

    - _(RandomAccessFile)_ raf - allows placing of the file descriptor at the needed offset

      - it is the __same__ for the all the tasks processing the same document

        - therefore, the access to this object is being __synchronized__ (1)

  - Reduce (in _main()_)

    - document name
    
    - the results list after the _Map_ stage
    
    - task id
      - used for the rank __equality__ cases

<!-- ------------------------------------------------------------------------------------ -->

## Parallelism

  Each thread is used for processing a list of _Map_ tasks, followed by a list of _Reduce_ tasks.

  Also, by using __semaphores__ (*), it is possible for a thread to process the _Reduce_ Task for a document if it has finished all the _Map_ tasks assigned to it.

<!-- ------------------------------------------------------------------------------------ -->

## Synchronization

  - (1) is used for avoiding a _race condition_ on the file descriptor at the _seek()_ method

  - (*) - each document has a __semaphore__ assigned to it (i.e. _semaphoreMap[docName]_)
    
    - it is initialised with `-numTasks + 1`, where _numTasks_ = no. tasks for the current document

    - after each _Map_ task ends -> _release()_
    - before each _Reduce_ task starts -> _acquire()_

<!-- ------------------------------------------------------------------------------------ -->


