Nume: Iancu Alexandru-Gabriel
Grupă: 333CB

# Tema 4

Organizare
-
1. Structura

Stochez toate structurile aferente planificatorului in structura Scheduler pe care
o folosesc prin intermediul unei instante globale numita sched. Aceasta contine
detalii relevante legate de starea planificatorului precum daca apelantul functiilor
e thread-ul root, prioritatea si alte informatii legate de thread-ul care este curent
pe procesor, timpul petrecut pe procesor de thread-ul curent si informatii despre noul
thread creat prin fork daca exista acesta. Mai are detalii cunoscute la initializare
cum ar fi numarul de evenimente si cuanta de timp a unui thread pe procesor. 
Are de asemenea si 2 semafoare pe care le folosesc pentru sincronizarea intre procesorul
nou creat cu fork si parintele sau.

Totusi cel mai important lucru este coada de prioritati in care stochez thread-urile aflate
in starea READY/RUNNING. Aceasta e conceputa din cate o lista inlantuita pentru fiecare din cele
6 prioritati posibile (0-5). In momentul in care se cauta urmatorul thread care trebuie planificat
se face de la lista cu prioritatea cea mai mare la cea cu prioritatea cea mai mica. Lista inlantuita
nevida cu prioritatea cea mai mare va da astfel urmatorul thread. Lista inlatuita este o lista circulara
in care se adauga la final noile thread-uri si se iau de la inceput urmatoarele thread-uri. Dupa ce s-a
luat urmatorul thread se shifteaza lista la stanga pentru ca acesta sa ajunga in coada listei.

O alta structura prezenta este thread_list care tine un istoric al tuturor thread-urilor create pentru
a avea la indemana id-urile thread-urilor cand se da join pe ele. Coada e insuficienta pentru ca nu
contine thread-urile aflate in WAITING. thread_list-ul contine si thread-urile TERMINATED, totusi
pthread_join-ul pe acestea se va termina rapid fara probleme.

Ultima structura din Scheduler este event_list care contine un vector cu informatii relevante despre un
event: mutex-ul si variabila de conditie folosite pentru sincronizarea intre thread-uri pe baza evenimentului
si o lista ce contine thread-urile ce asteapta ca evenimentul respectiv sa se intample.

Informatiile thread-ului sunt stocate intr-o structura ThreadInfo ce contine id-ul acestuia, un semafor
folosit pentru sincronizarea cu celelalte thread-uri si prioritatea acestuia care este rareori folosita.

2. Utilitatea temei

Tema a fost utila in aprofundarea folosirii bibliotecilor de linux si Windows pentru lucrul cu thread-uri in C.
Acuma sunt mai competent in sincronizarea thread-urilor pentru a evita aparitia de probleme de sincronizare
ce produc race conditions si bucle infinite. M-a ajutat helgrind ca sa descoper de unde venea nedeterminismul
in programul meu(era de la race conditions).

3. Eficienta temei

Implementarea cozii de prioritati este eficienta, aceasta in teorie adaugand un nou thread in O(1) si
facand rost de urmatorul thread tot in O(1) pentru ca parcurge un numar finit de liste inlantuite (6) pentru
a verifica daca sunt nevide, luand capul listei cu prioritatea cea mai mare. Eliminarea unui nod este O(n) unde
n este dimensiunea listei inlantuite din care face parte. Problema e ca in practica adaugarea nu este mereu O(1)
pentru ca in cazul in care thread-ul curent care este rulat are aceeasi prioritate ca thread-ul ce trebuie adaugat,
thread-ul nou adaugat trebuie pus pe antepenultima pozitie pentru ca la finalul rularii thread-ului curent pe procesor
acesta trebuie sa se afle la finalul listei. Asta induce in acea exceptie o complexitate de O(n) unde n este dimensiunea
listei in care este adaugat thread-ul pentru ca trebuie parcursa toata lista pentru a face rost de antepenultimul element.
O imbunatatire simpla este pastrarea in structura a elementului antepenultim care totusi ar scadea din lizibilitatea codului
si complexitatea (nu de timp) a acestuia, asta ar duce la avea mereu O(1) la adaugare.

Cand vine vorba complexitatea spatiala as zice ca nu este cea mai fericita implementare. Deseori am ales sa refolosesc
cod ce ocupa mai multa memorie decat aveam nevoie doar pentru lejeritate. Spre exemplu folosesc structura LinkedList pentru
istoricul thread_list desi am nevoie doar de id in loc si nu ar fi necesar sa aloc si semafoarele pentru structura
LinkedListNode. Daca as face-o eficient spatial ar trebui sa implementez o alta lista inlantuita doar cu id-ul sau
un vector dinamic ceea ce mi-ar fi ocupat niste timp si ar ingreuna lizibilitatea codului. De asemenea folosesc campul
priority din LinkedListNode intr-o singura situatie. Am incercat sa evit pana la final adaugarea acestuia pentru ca
voiam sa pastrez o incapsulare astfel incat prioritatea sa tina doar de coada de prioritati si nu de liste si noduri,
insa am dat de o problema pe care nu o puteam rezolva frumos altfel.

Implementare
-

Intregul enunt a fost realizat.

1. Flow

Implementarea incepe cu apelarea lui so_init() care aloca memorie pentru structurile folosite de planificator.
Dupa urmeaza primul fork care creeaza un thread ce porneste o functie start_thread in care e apelata
functia aferenta thread-ului nou creat. Inainte sa apeleze functia se fac niste sincronizari intre so_fork
si start_thread prin intermediul unor semafoare astfel incat thread-ul parinte sa il astepte pe copil sa intre
in starea READY/RUNNING si thread-ul copil sa astepte sa fie adaugat in coada de prioritati. Apoi thread-ul copil
asteapta la un semafor sa fie planificat. La finalul so_fork-ului se verifica daca thread-ul parinte mai poate
rula pe procesor sau nu, daca nu mai poate este replanificat alt thread printre cel copil daca au prioritatea mai
mare sau egala cu a lui(este incrementat semafor-ul corespondent thread-ului ce urmeaza sa fie planificat), dupa
asteapta si thread-ul parinte la semaforul lui sa fie replanificat. Partea aceasta de final se repeta la so_exec
si so_signal. In cazul in care cel care a apelat so_fork-ul e thread-ul root, atunci nu se mai fac verificari
si nici nu se mai asteapta la semafor in so_fork, doar se trezeste thread-ul ce urmeaza sa fie planificat.

Partea de so_wait and so_signal cu evenimente am facut-o folosindu-ma de variabile de conditie. Cei care fac
wait asteapta pe variabila de conditie asociata evenimentului la care asteapta, iar cei care fac signal dau
broadcast la cei care fac wait deja pe evenimentul respectiv. Dupa ce a dat lock pe mutex-ul variabilei de conditie,
inainte de a da wait, in so_wait thread-ul se va sterge din coada de prioritati si se adauga in lista de asteptare
pentru evenimentul respectiv apoi face yield si lasa alt thread sa fie planificat. In so_signal dupa ce thread-ul
face broadcast acesta va baga elementele din lista de asteptare a evenimentului in coada de prioritati si va goli
lista de asteptare. Dupa ce este trezit thread-ul, acesta se va bloca la semafor pana este replanificat. Nu ii mai
incrementez timpul pe procesor la wait cand se termina cu succes pentru ca oricum cand e replanificat i se va reseta
counter-ul.

2. Dificultati intampinate

-nu dadeam join si asa nu apucau thread-urile sa se termine si se inchidea programul si nu se dezaloca structura
thread-ului alocata de thread_create;
-la finalul executiei unui thread il scot din coada de prioritati, problema e ca asta era foarte probabil sa se
intample in timpul for-ului care parcurge coada de prioritati pentru a da join si aveam parte de race condition
pe coada de prioritati(lucru ce mi-a fost semnalat de helgrind); pentru a rezolva asta am folosit o lista separata
pentru a itera in for-ul de la join si anume thread_list;
-initial foloseam variabile de conditie pentru sincronizare la planificarea generala, o problema mare era ca se
putea intampla ca un thread sa dea signal inainte ca celalalt thread sa dea wait, lucru care ducea la o bucla infinita;
am rezolvat folosind semafoare binare pe care le initializam la valoarea 0 si astfel daca thread-ul care astepta
ajungea primul la sem_wait acesta astepta pana cineva ii dadea sem_post, iar daca cel carea dadea sem_post ajungea
primul valoarea devenea 1 si putea fi decrementata de sem_wait;
-cand adaugam un thread, cu aceeasi prioritate ca si thread-ul care e planificat curent, in lista cu prioritatea respectiva
din coada de prioritati, acesta era pus la final si astfel la terminarea thread-ului curent acesta nu era ultimul din lista
ci thread-ul nou adaugat, lucru ce era nedrept pentru thread-ul nou pentru ca thread-ul curent urma sa fie replanificat
inainte ca noul thread sa fie planificat; am rezolvat problema adaugand noul thread pe antepenultima pozitie din lista
asociata prioritatii.

Cum se compilează și cum se rulează?
-
* Linux - make sau make build pentru build si make clean pentru clean,
pentru a folosi loader-ul se adauga -lscheduler la gcc-ul
in care se compileaza propriul program, apoi se poate rula programul
* Windows - nmake sau nmake build pentru build si nmake clean pentru clean,
pentru a folosi loader-ul se adauga libscheduler.lib la cl-ul
in care se compileaza propriul program, apoi se poate rula programul

