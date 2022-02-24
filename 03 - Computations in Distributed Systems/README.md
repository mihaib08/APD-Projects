# APD - Tema 3 - Calcule colaborative in sisteme distribuite

Barbu Mihai-Eugen, 335CA [2021-2022]

----

## Implementare

- _tema3.cpp_ - fiecare proces contine variabilele:
    - `rank` - rangul procesului
    - `numTasks` - numarul de procese
    - `(std::vector<int *>) clusters` - informatiile legate
        de clustere (pentru aflarea topologiei)
        - fiecare element contine structura unui cluster

- _utils.cpp_ - contine functiile:
    - `getTopology()` - afiseaza in terminal topologia
                        corespunzatoare din ``clusters``
    - `displayMessage()` - mesajul logat in terminal
                           pentru fiecare operatie `MPI_Send()`

- _coordinator.cpp_ - contine functiile realizate de coordonatori,
                      in functie de prezenta
                      erorii de comunicatie (0/1):
    - `findTopology()`
    / `bonusTopology()` - aflarea topologiei
                          sistemului distribuit

    - `rootCompute()`
    / `rootBonusCompute()` - impartirea calculelor de la
                             procesul cu `rank = 0` catre
                             celelalte procese coordonator

    - `coordinatorCompute()`
    / `coordinator{1,2}Compute`
            - distribuirea calculelor de catre
              procesele coordonator cu $rank \in \{1,2\}$.

## Rezolvarea cerintelor

#### 1. Stabilirea topologiei

Fiecare proces coordonator citeste datele
din fisierul corespunzator,
dupa care creeaza clusterul `(int *) curr`
urmarindu-se regula stabilita:
- curr[0] - rangul coordonatorului
- curr[1] - numarul de procese worker
- curr[2..] - rangurile proceselor worker

Fiecare coordonator trimite celorlalti doi
informatiile clusterului sau (0 -> 1,2; 1 -> 0,2; 2 -> 0,1).

Dupa ce fiecare **coordonator** cunoaste
intreaga _topologie_, aceasta este trimisa
mai departe proceselor de tip **worker** aferente.

#### 2. Realizarea calculelor

Procesul cu `rank = 0` (_rootCompute()_)
_trimite_ numarul de elemente ale vectorului de procesat
celorlalte procese coordonator, genereaza vectorul
conform enuntului, dupa care:

1. se stabilesc limitele de calcul pentru procesele worker
2. pornind de la limita _superioara_ asignata ultimului proces worker,
   se trimit catre ceilalti coordonatori
   limitele inferioare pentru clusterele lor

    - `0` ii trimite lui `1` partea sa superioara,
      `1` trimite catre `0` partea sa superioara,
      aceasta fiind transmisa de `0` catre procesul `2`

3. trimite partile corespunzatoare de calculat
   catre workeri si ceilalti coordonatori
4. se asteapta rezultatele de la procesele worker
   si de la ceilalti coordonatori

Procesele coordonator `1` si `2` (_coordinatorCompute()_) primesc
`N` = numarul de elemente ale vectorului de procesat,
cat si limitele de start pentru asignarea
proceselor worker din cluster, dupa care:

1. se stabilesc limitele de calcul pentru procesele worker
2. se trimit limitele superioare ale clusterului
3. se primeste portiunea din vector alocata clusterului,
   dupa care se imparte conform limitelor stabilite
   la punctul 1 catre workeri
4. se asteapta rezultatele de la procesele worker
   si se trimit catre coordonatorul `0`

#### 4. Tratarea defectelor pe canalul de comunicatie

In cazul absentei legaturii `(0, 1)`, am considerat ca procesul `0`
va realiza comunicarea cu procesul `1` prin intermediul procesului `2`.
Astfel, comunicarea `0 <-> 1` devine `0 <-> 2 <-> 1`.

In acest mod, stabilirea topologiei se realizeaza astfel:

- procesele `0` si `1` trimit informatiile legate de
  clusterele sale catre `2`, dupa care
  coordonatorul `2` va trimite datele despre:
    - clusterele `0` si `2` catre procesul `1`
    - clusterele `1` si `2` catre procesul `0`

- procesele coordonator transmit topologia catre
  procesele worker

Pentru **realizarea calculelor**, procesul `0`
va trimite `v[curr..]` catre procesul `2`, unde
`curr` - limita superioara asignata proceselor worker
din clusterul `0`, dupa care `v[curr..]` va fi partitionat
intre procesele `1` si `2`.

----

### Obs.

- pentru procesele worker se realizeaza initial
  un `MPI_Recv()` avand parametrul **MPI_ANY_SOURCE**,
  dupa care rangul coordonatorului este extras din status.

- am folosit diverse tag-uri, in special la trimiterea
  consecutiva a mesajelor, pentru diferentierea continutului

- procesele _worker_ au **aceeasi** implementare indiferent
  de prezenta sau lipsa erorii de comunicatie, intrucat
  ele comunica **doar** cu procesul lor coordonator
