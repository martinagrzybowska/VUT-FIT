Dokumentácia k proj1 do IPK 2016/2017
=====================================

## NÁZOV
**ftrestd** - serverová časť klient/server aplikácie pre prenos súborov.

## OPIS
Serverová časť klient/server aplikácie pre prenos súborov, komunikujúca s klientskou časťou pomocou HTTP 1.1 protokolu, využívajúca RESTful API pre vybrané operácie.

## POUŽITIE
**ftrestd** [ **-r KOREŇOVÝ-ADRESÁR** ] [ **-p PORT** ]

## PREPÍNAČE
* **- r KOREŇOVÝ ADRESÁR** - špecifikuje koreňový adresár, do ktorého sú ukladané súbory pre jednotlivých užívateľov (implicitná hodnota je aktuálny adresár).
* **- p PORT** - špecifikuje port, na ktorom bude server naslúchať (implicitná hodnota je 6677).

## IMPLEMENTÁCIA

Serverová časť, implementovaná v jazyku C++ ako iteratívny TCP server, prijíma správu od klientskej časti, ktorá sa delí na HTTP hlavičku a dáta v binárnej podobe. Táto správa je následne spracovaná, pričom prebieha kontrola syntaktickej správnosti a dodržania pravidiel formátu hlavičky pre protokol HTTP 1.1. Zároveň s takouto kontrolou sú z prijatej správy extrahované informácie ako príkaz, vzdialená cesta či dĺžka bajtového obsahu, ktoré sú potrebné na vykonanie požadovaných operácii. Tieto informácie sú ukladané do príslušných komponentov obsahovej štruktúry pre budúce použitie. 

Po spracovaní obsahu správy server overuje typ objektu, na ktorý ukazuje cesta predaná cez parameter, pričom pomocou rady podmienok vyhodnocuje správnosť kombinácie požadovanej operácie nad týmto objektom. V prípade, že je daná kombinácia vyhlásená za valídnu, je volaná funkcia na vykonanie príslušnej operácie, inak sa vykonávanie operácie preskočí. Nasleduje volanie funkcie generujúcej odpoveď na dotaz od klienta, ktorej obsah záleží od úspechu či neúspechu vykonania požadovanej operácie, a takáto správa je klientskej časti následne odoslaná.


## CHYBOVÉ VÝPISY
Serverová časť odosiela prípadné chybové výpisy vzniknuté pri vykonávaní požadovaných operácii klientskej časti. 
Zoznam vlastných chybových výpisov servera spolu s návratovými hodnotami v zložených zátvorkách:
* **OPTS-ERR - Bad arguments** - chyba zadaných argumentov, nesprávny formát alebo počet. [3]
* **SOC-ERR - Something wrong with sockets** - chyba pri vytváraní soketu. [2]
* **BIND-ERR - Something wrong with bondage** - naviazania spojenia so soketom. [2]
* **LISTEN-ERR - Something wrong with listening** - chyba pri načúvaní. [2]


## PRÍKLADY POUŽITIA
```
$ ftrestd 
$ ftrestd -r xgrzyb00
$ ftrestd -p 6680
$ ftrestd -r xgrzyb00 -p 6680
$ ftrestd -p 6680 -r xgrzyb00
```

=====================================

## NÁZOV
**ftrest** - klientská časť klient/server aplikácie pre prenos súborov

## OPIS
Klientská časť klient/server aplikácie pre prenos súborov, komunikujúca so serverovou časťou pomocou HTTP 1.1 protokolu, využívajúca RESTful API pre vybrané operácie.

## POUŽITIE
**ftrest** **PRÍKAZ** **VZDIALENÁ-CESTA** [ **LOKÁLNA-CESTA** ]

## PREPÍNAČE
* **PRÍKAZ** - špecifikuje operáciu, ktorú má serverová časť vykonať.
* **VZDIALENÁ-CESTA** - udáva cestu k súboru alebo adresáru nachádzajúcom sa na serveri.
* **LOKÁLNA-CESTA**- udáva cestu v lokálnom súborovom systéme, tento prepínač je povinný pre príkaz **PUT**.

## PRÍKAZY
* **DEL** - zmaže súbor na serveri určený parametrom **VZDIALENÁ-CESTA**.
* **GET** - skopíruje súbor zo servera určený parametrom **VZDIALENÁ-CESTA** do aktuálneho adresára klienta alebo na miesto určené pomocou parametra **LOKÁLNA-CESTA** v prípade, že bol tento voliteľný parameter zadaný.
* **PUT** - skopíruje súbor zo servera určený parametrom **VZDIALENÁ-CESTA** na miesto určené parametrom **LOKÁLNA-CESTA**.
* **RMD** - odstráni adresár na serveri určený parametrom **VZDIALENÁ-CESTA**.
* **LST** - vypíše obsah adresára na serveri určeného parametrom **VZDIALENÁ-CESTA**.
* **MKD** - vytvorí adresár na serveri určený parametrom **VZDIALENÁ-CESTA**.

## IMPLEMENTÁCIA

Klientská časť spracováva vstupné parametre, z ktorých následne vytvára textovú reprezentáciu HTTP hlavičky. Pokiaľ sa jednalo o príkaz **PUT**, je k hlavičke pridaná aj binárna reprezentácia obsahu súboru, ktorý sa klient chystá preniesť na server. Po úspešnom odoslaní správy s požiadavkami a následnom prijatí odpovede klientská časť túto odpoveď začne spracovávať. Ak prijatá správa obsahuje potvrdenie o bezchybnom vykonaní operácie, klientská časť dokončí operáciu na svojej strane a úspešne sa ukončí. V opačnom prípade vytlačí na chybový výstup chybovú hlášku, ktorá sa nachádzala v tele správy prijatej od serverovej časti.
 
## CHYBOVÉ VÝPISY
Chybové výpisy je možné rozdeliť do dvoch nasledujúchich kategórii, ich návratové hodnoty sú uvedené v hranatých zátvorkách:
### 1. Vlastné chybové výpisy:
* **OPTS-ERR - Incorrect arguments** - jeden zo zadaných argumentov mal nesprávny tvar, alebo nepsprávny počet argumentov. [3]
* **CONNECT-ERR - Unable to set up connection** - nepodarilo sa nadviazať spojenie so serverovou časťou. [2]
* **CONNECT-ERR - Incorrect message form** - vytvorené správa nespĺňa požadovaný formát. [2]
* **CONNECT-ERR - Unable to sent header** - nepodarilo sa odoslať správu serverovej časti. [2]

### 2. Chybové výpisy prevzaté zo serverovej časti:
* **Not a directory.** - parameter **VZDIALENÁ-CESTA** ukazuje na súbor pri použití operácie je **LST** alebo **RMD**. [13]
* **Directory not found.** - parameter **VZDIALENÁ-CESTA** neukazuje na žiaden existujúci objekt pri použití operácii **LST** alebo **RMD**. [13]
* **Directory not empty.** - parameter **VZDIALENÁ-CESTA** ukazuje na adresár, ktorý nie je prázdny, pri použití operácie **RMD**.[13]
* **Already exists.** - parameter **VZDIALENÁ-CESTA** ukazuje na adresár/súbor, ktorý už existuje, pri použití operácie **MKD** alebo **PUT**. [13]
* **Not a file.** - parameter **VZDIALENÁ-CESTA** ukazuje na adresár, avšak vyžadovaná operácia je **DEL** alebo **GET**. [13]
* **File not found.** - parameter **VZDIALENÁ-CESTA** neukazuje na žiaden existujúci objekt pri použití operácii **DEL** alebo **GET**. [13]
* **User accound not found** - pokiaľ je vyžadovaná akákoľvek operácia nad neexistujúcim užívateľom. [13]
* **Unknown error** - špecifikuje všetky ostatné chyby pochádzajúce zo servera bez bližšieho popisu. [13]

## PRÍKLADY POUŽITIA
```
$ ftrest del http://localhost:6677/xgrzyb00/IPK17/ftrest.cpp
$ ftrest get http://localhost:6680/xgrzyb00/IPK17/readme.md 
$ ftrest put http://localhost:6680/xgrzyb00/IPK17/goodbye_note.pdf ~/final_will.pdf
$ ftrest rmd http://localhost:6677/xgrzyb00/IPK17
$ ftrest lst http://localhost:6690/xgrzyb00
$ ftrest mkd http://localhost:6690/xgrzyb00/coffin_orders.pdf
```

## AUTOR
**Martina Grzybowská** (**xgrzyb00**)
