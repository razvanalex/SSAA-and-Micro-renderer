------------------------------------------------------------------------------
                            TEMA 1 - APD
------------------------------------------------------------------------------

AUTOR:
  SMADU Razvan-Alexandru  335CB

FISIERE INCLUSE:
  - homework1.c
  - homework1.h
  - homework.c
  - homework.h
  - Readme.txt

README
  1. Super Sampling Anti-Aliasing
    Structuri de date:
      Structurile utilizate pentru a retine imaginea sunt:
        > colorPixel - retine valorile celor 3 culori ale unui pixel (3 octeti)
        > grayscalePixel - retine cantitatea de alb al unui pixel (1 octet)
        > image - retine metadata une imagini: 
                  - pType         poate fi P5 sau P6
                  - maxVal        valoarea maxima retinuta pentru culoare
                  - width         latimea in pixeli a imaginii
                  - height        inaltimea in pixeli a imaginii
                  - pixelMatrix   matricea de pixeli, implementare generica
        > threadFuncArgs - folosit pentru a transmite paramerii catre functiile
                           threadurilor. Pentru acest task se transmit id-ul
                           threadului, o referita a imaginii de input si una 
                           pentru imaginea de output.
      

    Functii si algoritmi:
      createMatrix(unsigned int width, unsigned int height, unsigned int sizeT)
        Creaaza o matrice de dimensiunile width x height, fiecare element avand
        dimensiunea sizeT, in memorie. Implementarea acestei functii este 
        generica pentru a putea crea o matrice atat pentru pixelii colorati 
        (de tipul colorPixel) cat si pentru pixelii alb-negru (de tipul 
        grayscalePixel). In cazul in care alocarea esueaza, se returneaza NULL.

      destroyMatrix(void*** matrix, unsigned int height)
        Elibereaza memoria unei matrice, de inaltime height. Implementarea este 
        independenta de tipul elementelor. La final, matricea va pointa la NULL.

      createPixelMatrix(image *img)
        Creeaza matricea de pixeli, in functie de tipul acesteia. Pentru o 
        imagine color (tip P6), matricea va avea elemente de tipul colorPixel, 
        iar pentru o imagine alb-negru (tip P5), matricea va avea elemente de 
        tipul grayscalePixel. Functia returneaza 0 pentru esec si 1 pentru 
        succes.
      
      readInput(const char * fileName, image *img)
        Reprezinta functia din scheletul de cod si realizeaza citirea imaginii
        din fisier. In primul rad, se deschide fisierul pentru citire, se 
        "curata" structura imaginii setand toti bitii la 0, apoi se citesc 
        informatiile despre imagine (tip, dimensiune, valoare maxima pentru 
        pixel), iar apoi, in functie de tipul imaginii, se citesc valorile 
        pixelilor. La final se inchide fisierul pt citire. Durata de executie a 
        functiei este proportionala cu dimensiunea imaginii. Pentru imaginea 
        color, se citeste in blocuri de 3 octeti (adica un pixel color) pentru 
        a creste performantele la citire. Acest lucru este posibil deoarece nu 
        exista padding in structura, intre cele 3 culori; insa acest lucru poate 
        sa depinda de compilator (gcc nu pune padding).

      writeData(const char * fileName, image *img)
        Realizeaza scrierea in fisier a imaginii date ca parametru. Aceasta 
        functie este simetrica cu readInput(), operatiile fiind de scriere
        (fwrite si fprintf) in loc de citire (fscanf si fread). La final insa,
        se elibereaza memoria imaginii pentru a evita memory leak-uri.
      
      computeGrayscale(image *out, unsigned int x, unsigned int y, image *img)
        si
      computeColor(image *out, unsigned int x, unsigned int y, image *img)
        Aceste functii calculeaza culorile noilor pixeli in imaginea micsorata
        de resize_factor ori. Se calculeaza pixelii din vechea imagine care
        contribuie la culoarea pixelului din noua imagine conform urmatoarelor
        formule:
            startX = x * resize_factor
            endX = (x + 1) * resize_factor - 1
            startY - y * resize_factor
            endY = (y + 1) * resize_factor - 1
          unde  startX - pozitia pe X a primului pixel care contribuie la noua 
                         culoare
                startY - pozitia pe Y a primului pixel care contribuie la noua
                         culoare
                endX   - pozitia pe X a ultimului pixel care contribuie la noua
                         culoare
                endY   - pozitia pe Y a ultimului pixel care contribuie la noua
                         culoare
          
        Pentru un resize_factor par, media aritmetica a pixelilor din regiunea 
        [startX, endX] x [startY, endY] reprezinta noua culoare, iar pentru
        resize_factor = 3 se calculeaza aplicand nucleul gaussian asupra 
        acestor pixeli (se inmulteste fiecare element cu elementul 
        corespunzator din kernelul gaussian, se aduna numerele rezultate si 
        apoi se imparte la 16)

        De remarcat ca in zona [startX, endX] x [startY, endY] se afla 
        resize_factor * resize_factor pixeli.
        In cazul imaginilor color, se calculeaza fiecare noua culoare separat
        pe R, G si B.

        Acest calcul nu este foarte intensiv din punct de vedere hardware si 
        depinde, ca si complexitate, de resize_factor, care, in general,
        nu este un numar foarte mare. Prin urmare aceasta functie nu este 
        paralelizata, insa va fi executata de fiecare thread, pentru a 
        calcula noii pixeli.

        Cele doua functii realizeaza acelasi task, diferenta constand in numarul
        de culori prelucrate.

      intervalStart(int N, int P, int Tid)
        Aceasta functie calculeaza in O(1) pozitia de start al primului element
        al unui grup, in care impartirea a N elemente s-a facut in P grupuri.
        Identificarea grupului se realizeaza prin Tid. Folosind aceasta functie,
        se repartieaza fiecarui thread o zona de memorie, aproximativ egala 
        intre threaduri, care urmeaza a fi prelucrata. Aceasta functie 
        balanseaza impartirea astfel incat numarul de elemente intre grupuri 
        sa varieze cu cel mult 1. Acest lucru va face ca threadurile sa aiba 
        timpi apropiati de executie.
        Impartirea este urmatoarea: primele N % P grupuri vor avea cu un 
        element mai mult decat restul grupurilor. De exemplu pentru N = 10 si 
        P = 4 vom avea urmatoarea impartire:
                                0 1 2 3 4 5 6 7 8 9
                                \___/ \___/ \_/ \_/
                    Id grup:      0     1    2   3
        Dupa cum se poate observa dimensiunile grupurilor sunt: 3 3 2 2.

      threadFunc(void* args)
        Aceasta functie este cea rulata de fiecare thread in parte si ruleaza
        algoritmul paralel. Sunt extrase argumentele din structura, 
        se liniarizeala matricea pentru o impartire cat mai eficienta intre 
        threaduri, si pentru fiecare pixel al threadului respectiv se calculeaza
        culoarea pixelului imaginii de output.

        Liniarizand matricea, se poate imparti, de exemplu, si o imagine ce 
        contine o singura linie foarte lunga intre mai multe threaduri.

        Trecerea de la vector linie la matrice se face prin variabilele x si y
        care sunt calculate in functie de indexul pixelului si lungimea liniei.
        In cazul in care imaginea este alb-negru, se aplica computeGrayscale(),
        iar pentru imaginea color se aplica computeColor(). Detalii despre 
        algoritmul paralel vor fi abordate in sectiunea <Paralelizarea 
        algoritmului>.

      resize(image *in, image *out) 
        Aceasta functie creeaza imaginea de iesire (seteaza metadatele si 
        se aloca memorie pentru matricea de pixeli), creeaza fire de executie
        care ruleaza algoritmul paralel, iar apoi se elibereaza memoria
        care nu va mai fi folosita pe viitor pentru a evita memory leak-uri.
        Timpul de rulare al acestei functii depinde foarte mult si de timpul 
        pentru a aloca memoria si a o elibera (intrucat este realizata de 
        sistemul de operare) costul paralelizarii ar putea sa nu aduca niciun
        beneficiu pe sistemele care pun lock pe memorie atunci cand se aloca.


    Paralelizarea algoritmului
      Dandu-se o imagine foarte mare, se doreste reducerea dimensiunii ei 
      intr-un mod cat mai eficient pe sistemele multi-thread. 
      Pentru cazul in care resize_factor este multiplu de 2, se ia un patrat de 
      pixeli care reprezinta noul pixel si se face media aritmetica a acelor
      pixeli. Intrucat acest lucru nu impune decat cel mult citiri dintr-o 
      zona de memorie, si scrieri in zone total diferite pentru fiecare pixel in
      parte, operatiile realizate sunt thread-safe si nu vor aparea probleme 
      de concurenta. Prin urmare, o impartire cat mai echitabila a pixelilor din
      matrice poate face ca threadurile sa fie folosite cat mai eficient 
      posibil. Deci, algoritmul paralel este urmatorul:

        int num_threads
        image in, out

        co [p = 1 to num_threads] {
          int x, y
          int numPixels = out.width * out.height
          int start = intervalStart(numPixels, num_threads, p)
          int end = intervalStart(numPixels, num_threads, p + 1) - 1

          for [i = start to end] {
            x = i % out.width
            y = i / out.width
            if (out.pType == '5')
              call computeGrayscale(out, x, y, in)
            else 
              call computeColor(out, x, y, in)
          }
        }

      Din algoritm reiese faptul ca fiecare fir de executie isi calculeaza 
      inceputul si sfarsitul in functie de id-ul threadului, iar structura 
      pentru imagine, respectiv functiile folosite sunt cele din codul in C 
      (sau foarte asemanatoarea cu acestea).

      Complexitatea algoritmului secvential este 
        O(out.height * out.width * resize_factor^2) = O(in.height * in.width)
      
      Complexitatea algoritmului paralel, cand numarul de fire de executie este 
      egal cu numarul de pixeli din imaginea de output, este:
        O(resize_factor^2)

      In cazul in care numarul de threaduri este mai mic decat numarul de pixeli
      complexitatea este O((out.height * out.width * resize_factor^2) / P), 
      

    Scalabilitate
      In continuare, voi prezenta cum se reflecta aceste complexitati asupra 
      timpilor de executie, pe input-uri de diferite dimensiuni si rularea 
      pe un numar diferit de threaduri.

      Nota: Teste au fost realizate pe cluster-ul facultatii in coada
            ibm-nehalem.q

      Testarea s-a realizat prin intermediul unui script care ruleaza programul
      pe mai multe configuratii. Output-ul acestuia are urmatoarea semnificatie:
      - Cuvantul "Test" - primul cuvat
      - numele imaginii - al doilea cuvant
      - tipul imaginii (bw - black and white; color - color) - al treilea cuvant
      - resize_factor - al 4lea cuvant 
      - dupa puncte (..........) urmeaza timpul afisat de program (afisarea din 
        scheletul de cod).
      Timpii de citire si scriere nu sunt luati in calcul.

      Imaginile utilizate sunt: 
        - lenna -> imaginea din schelet de dimensiunea 512x512
        - test2 -> imagine de dimensiunea 8032x4520 (varianta color are ~109MB)
        - test1 -> imagine de dimensiunea 15360x8640 (varianta color are ~400MB)

      Rulare pe 1 thread:
        ============== Exercise 1 ==============
        Test lenna bw 2         .......... 0.006135
        Test lenna bw 3         .......... 0.004995
        Test lenna bw 8         .......... 0.003046
        Test lenna bw 16        .......... 0.002826
        Test lenna color 2      .......... 0.008649
        Test lenna color 3      .......... 0.009067
        Test lenna color 8      .......... 0.005026
        Test lenna color 16     .......... 0.004759
        Test test2 bw 2         .......... 0.470315
        Test test2 bw 3         .......... 0.352171
        Test test2 bw 8         .......... 0.231464
        Test test2 bw 16        .......... 0.223850
        Test test2 color 2      .......... 0.731902
        Test test2 color 3      .......... 0.727827
        Test test2 color 8      .......... 0.465303
        Test test2 color 16     .......... 0.406079
        Test test1 bw 2         .......... 1.643984
        Test test1 bw 3         .......... 1.524104
        Test test1 bw 8         .......... 0.749222
        Test test1 bw 16        .......... 0.724065
        Test test1 color 2      .......... 2.437840
        Test test1 color 3      .......... 2.687723
        Test test1 color 8      .......... 1.452959
        Test test1 color 16     .......... 1.495601

      Rulare pe 2 thread:
        ============== Exercise 1 ==============
        Test lenna bw 2         .......... 0.003217
        Test lenna bw 3         .......... 0.002709
        Test lenna bw 8         .......... 0.001477
        Test lenna bw 16        .......... 0.001605
        Test lenna color 2      .......... 0.004660
        Test lenna color 3      .......... 0.004778
        Test lenna color 8      .......... 0.002443
        Test lenna color 16     .......... 0.002562
        Test test2 bw 2         .......... 0.268784
        Test test2 bw 3         .......... 0.178014
        Test test2 bw 8         .......... 0.100305
        Test test2 bw 16        .......... 0.129219
        Test test2 color 2      .......... 0.393093
        Test test2 color 3      .......... 0.413989
        Test test2 color 8      .......... 0.249526
        Test test2 color 16     .......... 0.201094
        Test test1 bw 2         .......... 0.869570
        Test test1 bw 3         .......... 0.774045
        Test test1 bw 8         .......... 0.478363
        Test test1 bw 16        .......... 0.449654
        Test test1 color 2      .......... 1.525035
        Test test1 color 3      .......... 1.357158
        Test test1 color 8      .......... 0.900120
        Test test1 color 16     .......... 0.808932

      Rularea pe 8 threaduri:
        ============== Exercise 1 ==============
        Test lenna bw 2         .......... 0.002197
        Test lenna bw 3         .......... 0.001004
        Test lenna bw 8         .......... 0.000719
        Test lenna bw 16        .......... 0.000669
        Test lenna color 2      .......... 0.002694
        Test lenna color 3      .......... 0.001537
        Test lenna color 8      .......... 0.000906
        Test lenna color 16     .......... 0.000976
        Test test2 bw 2         .......... 0.090850
        Test test2 bw 3         .......... 0.055067
        Test test2 bw 8         .......... 0.027371
        Test test2 bw 16        .......... 0.023014
        Test test2 color 2      .......... 0.116839
        Test test2 color 3      .......... 0.087208
        Test test2 color 8      .......... 0.079393
        Test test2 color 16     .......... 0.091295
        Test test1 bw 2         .......... 0.243787
        Test test1 bw 3         .......... 0.212565
        Test test1 bw 8         .......... 0.130946
        Test test1 bw 16        .......... 0.115243
        Test test1 color 2      .......... 0.418418
        Test test1 color 3      .......... 0.414624
        Test test1 color 8      .......... 0.265851
        Test test1 color 16     .......... 0.257984

      Dupa cum se poate observa din valorile anterioare, cu cat este mai mare 
      numarul de thread-uri, cu atat timpul de executie este mai mic. Se 
      constata la test1 color o diferenta de aproape 6 ori intre rularea pe un 
      singur thread si cea pe 8 thread-uri.


  2. Micro renderer
    Pentru realizarea acestui task, multe functii au fost preluate din 
    rezolvarea task-ului 1, si adaptate pe nevoile cerute.

    Structuri de date:
      > threadFuncArgs - contine doar id-ul threadului si imaginea de output
      > image - aceeasi ca cea de la task-ul 1; cu diferenta ca pType este un 
                char[2], si nu se retine si '\0' la final.
      > grayscalePixel - aceeasi ca cea de la task-ul 1

    Functii si algoritmi:
      createMatrix(unsigned int width, unsigned int height, unsigned int sizeT)
        Aceeasi functionalitate ca la task-ul 1.
      
      destroyMatrix(void*** matrix, unsigned int height)
        Aceeasi functionalitate ca la task-ul 1.
      
      createPixelMatrix(image *img)
        Aceeasi functionalitate ca la task-ul 1, insa functioneaza doar pentru 
        imagine alb-negru.
      
      initialize(image *im)
        Initializeaza proprietatile imaginii si creeaza matricea de pixeli
      
      distanceFunc(double x, double y)
        Calculeaza dinstanta dintre un punct cu coordonatele (x, y) si dreapta 
        de ecuatie -x + 2y = 0. De remarcat faptul ca foloseste abs() pentru 
        calculul valorii absolute.
      
      intervalStart(int N, int P, int Tid)
        Aceeasi functionalitate ca la task-ul 1.
        
      threadFunc(void* args)
        Aceasta functie este cea executata de fiecare thread in parte. In prima 
        faza, se extrag argumentele din structura, se calculeaza constantele
        necesare si, pentru fiecare pixel in parte, se verifica daca pozitia 
        acestuia este la distanta mai mica sau egala cu 3. In acest caz, se 
        coloreaza pixelul in negru, altfel in alb. Centrul pixelului este 
        calculat ca fiind produsul dintre dimensiunea pixelului si indicele 
        pixelului adunat cu 0.5. Axa Y trebuie inversata, pentru ca punctul 
        (0,0) sa fie in coltul din stanga-jos. Liniarizarea matricei se face si 
        la acest task pentru a putea folosi thread-urile eficient. Ideea de 
        grupare a pixelilor este aceeasi ca la task-ul 1. Complexitatea, in 
        cazul de fata este dat de numarul de pixeli din imagine, cu alte cuvinte
        O(im.width * im.height), deoarece restul calculelor sunt atomice se se 
        realizeaza in O(1).

      render(image *im)
        Creeaza fire de executie, executa functia threadFunc() si la final 
        elibereaza memoria nefolosita. Timpul de executie poate fi influentat de 
        sistemul de operare in momentul in care se elibereaza memoria.

      writeData(const char * fileName, image *img)
        Aceasta functie scrie imaginea in fisierul de output, respectand 
        formatul unui fisier pgm.


    Paralelizarea algoritmului
      Acest task presupune luarea fiecarui pixel in parte, si verificarea 
      acestuia daca respecta conditia de a fi la distanta 3 de dreapta. 
      Prin urmare, calculul pentru fiecare pixel este independent de ceilalti
      pixeli si se poate paraleliza dupa acest criteriu. Deci, am 
      liniarizat matricea, si am impartit-o in num_threads fire de executie.
      Fiecare thread verifica daca pixelii pe care ii are de prelucrat 
      respecta conditiile. Algoritmul paralel este:

        int num_threads
        image img

        co [p = 1 to num_threads] {
          int x, y, i, j
          int pixel

          real pixelWidth = 100 / img.height
          int numPixels = img.height * img.width
          int start = intervalStart(numPixels, num_threads, p) 
          int end = intervalStart(numPixels, num_threads, p + 1) - 1
          
          for [pixel = start ot end] {
            i = pixel / im.width
            j = pixel % im.width
            x = pixelWidth * (j + 0.5)
            y = pixelWidth * (im.height - i - 0.5)

            if (distanceFunc(x, y) <= 3)
              call colorPixelWhite(img, i, j)
            else 
              call colorPixelBlack(img, i, j)
          }
        }

      In acest pseudocod, am folosit functiile si notatiile din cod. Functiile 
      colorPixelBlack() primeste imaginea si pozitia pixelului si il coloreaza 
      in negru, iar colorPixelWhite() primeste aceiasi parametrii si coloreaza 
      pixelul in alb.

      Se poate observa ca daca numarul de threaduri este  egal cu numarul de 
      pixeli, atunci complexitatea algoritmului devine O(1). Complexitatea 
      algoritmului secvential este O(img.width * img.height).


    Scalabilitate
      In continuare, voi prezenta modul cum algoritmul este scalabil in raport 
      cu numarul de thread-uri.

      Nota: Teste au fost realizate pe cluster-ul facultatii in coada
            ibm-nehalem.q

      Testarea s-a realizat prin intermediul unui script care ruleaza programul
      pe mai multe configuratii. Output-ul acestuia are urmatoarea semnificatie:
      - Cuvantul "Test" - primul cuvant
      - Un numar ce reprezinta rezolutia imaginii
      - Dupa puncte (..........) urmeaza timpul afisat de program (afisarea din 
        scheletul de cod).

      Rularea pe 1 thread
        ============== Exercise 2 ==============
        Test 10         .......... 0.000226
        Test 1000       .......... 0.067783
        Test 2000       .......... 0.180743
        Test 5000       .......... 0.962195
        Test 8000       .......... 2.487703
        Test 10000      .......... 3.893797
   
      Rularea pe 2 threaduri
        ============== Exercise 2 ==============
        Test 10         .......... 0.000171
        Test 1000       .......... 0.020823
        Test 2000       .......... 0.092497
        Test 5000       .......... 0.481810
        Test 8000       .......... 1.239109
        Test 10000      .......... 1.950566

      Rularea pe 8 threaduri
        ============== Exercise 2 ==============
        Test 10         .......... 0.000311
        Test 1000       .......... 0.005184
        Test 2000       .......... 0.019551
        Test 5000       .......... 0.120990
        Test 8000       .......... 0.313066
        Test 10000      .......... 0.488015

      Se poate observa ca pentru un input mic, durata de creere a thread-urilor 
      este mult mai mare decat executia algoritmului propriu-zis. Insa la 
      un input mare (e.g. 10000) diferenta dintre 1 thread si 8 thread-uri este 
      de aproape 8 ori, iar dintre 1 thread si 4 thread-uri este de aproape 
      4 ori.

