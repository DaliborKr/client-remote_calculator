# CHANGELOG
## **IPK - Projekt 1: IPK Calculator Protocol**
#### Autor: `Dalibor Kříčka`
#### Login: `xkrick01`
#### Brno 2023
---

## **Stručný popis implementované funkcionality**

Program po ověření a zpracování argumentů programu (_IPv4 adresa_, _port_, _režim_) vytvoří socket podle typu uživatelem zvoleného režimu pomocí funkce `socket()`. Dále nastaví zadané informace o serveru, aby s ním bylo možné komunikovat pomocí struktur `sockaddr` a `sockaddr_in`. Následně čte zprávy po řádcích ze standardního vstupu pomocí funkce `getline()`. Tyto zprávy poté, pokud je zvolený režim _UDP_, náležitě upraví (přidá operační kód a délku přenášených dat). Následně zprávu odešle serveru a čeká na odpověď, kterou vypíše na standardní výstup. V případě režimu _TCP_ se před komunikací naváže spojení se serverem, které se po skončení komunikace zase řádně ukončí. UML diagram aktivit je k nalezení v [README.md](README.md).

## **Limitace**

Program byl testován na operačních systémech `NixOS`, `Fedora` a `CentOS`. Na jiných než výše uvedených operačních systémech tedy není zaručena korektní funkcionalita (Windows, MacOS, ...). Konkrétně systém Windows ve své standardní knihovně neobsahuje hlavičkové soubory `sys/socket.h` a `arpa/inet.h` a bylo by tak třeba použít nějakou alternativu (např. `winsock2.h`).

**Maximální** počet Bytů, který může mít zpráva od zpráva přijatá od serveru (včetně operačního kódu, stavového kódu i délky posílané zprávy), je **1024B**.