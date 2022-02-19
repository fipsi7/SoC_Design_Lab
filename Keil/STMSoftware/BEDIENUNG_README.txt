Voraussetzung: Hauptplatine (interner STM) ist mit Hauptplatinensoftware geflashed
PC mit externem STM verbinden
Keil starten
Programm flashen
Terminal öffnen
Uartverbindung: 9600 Baud, 8 Datenbits, 1 Stopbits, Parity None, keine Flow Control

ASCII-Zeichen versenden:
  LinA steuern:
    a: up	
    s: stop
    d: down
  LinB steuern:
    q: up
    w: stop
    e: down
  MotA steuern:
    r: Stein down
    t: Stein up
    f: Motor lösen
    g: Motor halten
  MotB steuern:
    o: close
    l: open