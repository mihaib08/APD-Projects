# APD - Tema 1 - Paralelizarea unui algoritm genetic

Barbu Mihai-Eugen, 335CA [2021]

----

## Implementare

Pornind de la fisierele sursa din _skel/_, am adaugat urmatoarele 
functionalitati pentru paralelizarea calculelor:

- ```struct th_arg``` (_utils.h_) - structura folosita pentru variabilele
                                    pasate ca argument la lansarea thread-urilor:
    - id - indexul thread-ului
    - *current_generation - generatia curenta
    - *next_generation - generatia viitoare
    - generations_count - numarul de iteratii pentru
                          formarea generatiilor
    - object_count / *objects - obiectele / numarul de obiecte
    - sack_capacity - capacitatea rucsacului

    - P - numarul de thread-uri
    - *barrier - **pointer** catre bariera folosita la
                 sincronizarea thread-urilor
    
    - *st, *en - vectori care pastreaza _indicii_
                 corespunzatori paralelizarii
                 pe fiecare thread - utile pentru Obs. 1)

Am definit functia ```thread_genetic()```, aceasta preluand initializarea 
celor doi vectori si cele _generations\_count_ iteratii din _run\_genetic\_algoritm()_
din varianta _secventiala_, pe care le-am paralelizat astfel:

- pornind de la indexul unui thread, _id_, si de la numarul de thread-uri _P_,
  se calculeaza indicii de iteratie in vectorii
  _*current\_generation_ si _*next\_generation_
    - _start_, _end_ - in functie de _object\_count_
    - _s_, _e_ - in functie de _count_

- se foloseste bariera **barrier** pentru a sincroniza thread-urile la:

    - initializarea vectorilor
    - sortarea pe bucati a lui _*current\_generation_
    - interclasarea subvectorilor sortati obtinuti
    - operatiile pentru formarea noii generatii
        - am delimitat verificarea paritatii numarului
          de parinti pentru a se face copierea pentru
          urmatoarea generatie doar pe un thread

### Obs.

1)  Pentru sortarea indivizilor, am plecat de la
    functia ```qsort()```
    pe care am paralelizat-o, dupa care am interclasat subvectorii rezultati.

    Am considerat ca aceasta abordare este mai buna, intrucat are complexitatea
    **O(N/P * log(N/P) + N * P)**,
    in timp ce abordarea cu OETS
    ar fi presupus _O(N * N/P)_.

    Pentru _Merge-Sort_ paralel, m-am gandit la a face padding cu 0 pe vector,
    dar ar putea conduce la mai multe operatii in functie de N
    (ex: N = 1030 -> s-ar face padding pana la 2048 -> >1000 de elemente).

2) In cazul afisarii am folosit vectorul _*next\_generation_,
   intrucat dupa sortare am interclasat valorile in acesta,
   dupa care tranzitia la generatia urmatoare se realizeaza direct in _*current\_generation_.
   Astfel, in cazul gasirii celui mai bun fitness,
   valorile din _*current\_generation_ din implementarea **secventiala** 
   sunt **echivalente** cu cele din 
   _*next\_generation_ din implementarea **paralela**.
