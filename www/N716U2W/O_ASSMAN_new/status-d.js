//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

tabArray=['Druckservers','Status','Installation','Sonstiges','Neustart','System','Drucker','Drahtlos','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
//Language : Deutsch

//system.htm
headArray[iIndex++] = "<BR>Auf dieser Seite werden die allgemeinen Systemdaten des Druckservers angezeigt.<BR>";
//printer.htm
headArray[iIndex++] = "<BR>Auf dieser Seite werden die Daten des Druckers angezeigt, der gegenw�rtig an den Druckserver angeschlossen ist.<BR>Hinweis: Wenn Ihr Drucker nicht bidirektional funktioniert, ist es m�glich, dass einige Daten nicht richtig wiedergegeben werden.";
//wlan.htm
headArray[iIndex++] = "<BR>Auf dieser Seite werden die aktuellen Drahtlos-Einstellungen und status des Druckservers angezeigt.";
//tcpip.htm
headArray[iIndex++] = "<BR>Auf dieser Seite werden die aktuellen TCP/IP-Einstellungen des Druckservers angezeigt.<BR>";
//netware.htm
headArray[iIndex++] = "<BR>Auf dieser Seite werden die aktuellen NetWare-Einstellungen des Druckservers angezeigt.<BR>";
//apple.htm
headArray[iIndex++] = "<BR>Auf dieser Seite werden die aktuellen AppleTalk-Einstellungen des Druckservers angezeigt.<BR>";
//snmp.htm
headArray[iIndex++] = "<BR>Auf dieser Seite werden die aktuellen SNMP-Einstellungen des Druckservers angezeigt.<BR>";
//Smb.htm
headArray[iIndex++] = "<BR>Auf dieser Seite werden die Einstellungen f�r die gemeinsame Druckerverwendung bei Microsoft Windows-Netzwerken angezeigt.<BR>";
iIndex = 0;



//system.htm
textArray0[iIndex++]="Systemdaten";
textArray0[iIndex++]="Name des Druckservers :";
textArray0[iIndex++]="Systemkontakt :";
textArray0[iIndex++]="Systemposition :";
textArray0[iIndex++]="Systembetriebszeit :";
textArray0[iIndex++]="Firmwareversion :";
textArray0[iIndex++]="MAC-Adresse :";
textArray0[iIndex++]="Alarm per e-mail :";
textArray0[iIndex++]="deaktiviert";
textArray0[iIndex++]="aktiviert";
//PRINTJOB.htm
textArray0[iIndex++]="Druckauftr�ge";
textArray0[iIndex++]="Auftrag";
textArray0[iIndex++]="Benutzer";
textArray0[iIndex++]="Verstrichene Zeit";
textArray0[iIndex++]="Protokoll";
textArray0[iIndex++]="Port";
textArray0[iIndex++]="Status";
textArray0[iIndex++]="Gedruckte Bytes";
textArray0[iIndex++]="Auftragsprotokoll anzeigen";
iIndex = 0;

// PRINTER.HTM
textArray1[iIndex++]="Daten Drucker";
textArray1[iIndex++]="Hersteller";
textArray1[iIndex++]="Modellnummer";
textArray1[iIndex++]="Unterst�tzte Drucksprache";
textArray1[iIndex++]="Aktueller Status";
textArray1[iIndex++]="OK";
textArray1[iIndex++]="Kein Papier";
textArray1[iIndex++]="Offline";
textArray1[iIndex++]="Belegt";
iIndex = 0;

//NETWARE.htm
textArray2[iIndex++]="Allgemeine Einstellungen";
textArray2[iIndex++]="Name des Druckservers :";
textArray2[iIndex++]="Polling-Zeit :";
textArray2[iIndex++]="Sekunden";
textArray2[iIndex++]="NDS-Einstellungen von NetWare";
textArray2[iIndex++]="NDS-Modus verwenden :";
textArray2[iIndex++]="deaktiviert";
textArray2[iIndex++]="aktiviert";
textArray2[iIndex++]="Name des NDS-Baums :";
textArray2[iIndex++]="Name des NDS-Kontexts :";
textArray2[iIndex++]="Aktueller Status :";
textArray2[iIndex++]="deverbunden";
textArray2[iIndex++]="verbunden";
textArray2[iIndex++]="NetWare-Bindery-Einstellungen";
textArray2[iIndex++]="Bindery-Modus verwenden :";
textArray2[iIndex++]="deaktiviert";
textArray2[iIndex++]="aktiviert";
textArray2[iIndex++]="Name des Dateiservers :";
textArray2[iIndex++]="Aktueller Status :";
textArray2[iIndex++]="deverbunden";
textArray2[iIndex++]="verbunden";
iIndex = 0;
//tcpip.htm
textArray3[iIndex++]="TCP/IP-Einstellungen";
textArray3[iIndex++]="DHCP/BOOTP verwenden :";
textArray3[iIndex++]="IP-Adresse :";
textArray3[iIndex++]="Subnet-Mask :";
textArray3[iIndex++]="Gateway :";
//randvoo.htm
textArray3[iIndex++]="Rendezvous-Einstellungen";
textArray3[iIndex++]="Rendezvous-Einstellungen :";
textArray3[iIndex++]="deaktiviert";
textArray3[iIndex++]="aktiviert";
textArray3[iIndex++]="Dienstname :";
iIndex = 0;

// APPLE.HTM
textArray4[iIndex++]="AppleTalk-Einstellungen";
textArray4[iIndex++]="AppleTalk :";
textArray4[iIndex++]="Druckerdaten";
textArray4[iIndex++]="Portname :";
textArray4[iIndex++]="Druckertyp :";
textArray4[iIndex++]="Datenformat :";
iIndex = 0;

// SNMP.HTM
textArray5[iIndex++]="SNMP-Community-Einstellungen";
textArray5[iIndex++]="SNMP-Community 1 :";
textArray5[iIndex++]="Nur Lesen";
textArray5[iIndex++]="Lesen und Schreiben";
textArray5[iIndex++]="SNMP-Community 2 :";
textArray5[iIndex++]="Nur Lesen";
textArray5[iIndex++]="Lesen und Schreiben";
textArray5[iIndex++]="SNMP-Trap-Einstellungen";
textArray5[iIndex++]="SNMP-Traps senden :";
textArray5[iIndex++]="deaktiviert";
textArray5[iIndex++]="aktiviert";
textArray5[iIndex++]="Authentifizierungs-Traps verwenden :";
textArray5[iIndex++]="deaktiviert";
textArray5[iIndex++]="aktiviert";
textArray5[iIndex++]="Trap-Adresse 1 :";
textArray5[iIndex++]="Trap-Adresse 2 :";
iIndex = 0;

//PRINTJOB.htm
// Translate                                  only "Refresh " is to be translated
textArray6[iIndex++]='<input type=button value=" Aktualisieren " onClick="window.location.reload()">';
textArray6[iIndex++]="Druckauftr�ge";
textArray6[iIndex++]="Auftrag";
textArray6[iIndex++]="Benutzer";
textArray6[iIndex++]="Verstrichene Zeit";
textArray6[iIndex++]="Protokoll";
textArray6[iIndex++]="Port";
textArray6[iIndex++]="Status";
textArray6[iIndex++]="Gedruckte Bytes";
// Translate                                  only "Close " is to be translated
textArray6[iIndex++]='<input type=button value=" Schlie�en " onClick="window.close()">';
iIndex = 0;

//SMB.htm
textArray7[iIndex++]="Workgroup";
textArray7[iIndex++]="Name :";
textArray7[iIndex++]="Gemeinsam genutzter Name";
textArray7[iIndex++]="Drucker :";
iIndex = 0;

//WLAN.htm
textArray8[iIndex++]="<BR><BR><FONT COLOR=RED>Warnung! Der Druckerserver befindet sich derzeit im Diagnostic Mode.</FONT><BR>";
textArray8[iIndex++]="Drahtlos-Informationen";
textArray8[iIndex++]="Modus :";
textArray8[iIndex++]="Netzknoten-Modus (Client mode)";
textArray8[iIndex++]="AP-Modus (Access Point mode)";
textArray8[iIndex++]="Modi betrieben werden :";
textArray8[iIndex++]="Infrastruktur-Modus";
textArray8[iIndex++]="Ad-hoc-Modus";
textArray8[iIndex++]="AP-MAC-Adresse :";
textArray8[iIndex++]="SSID :";
textArray8[iIndex++]="Kanal :";
textArray8[iIndex++]="Norm (Standard) :";
textArray8[iIndex++]="Nur 802.11b";
textArray8[iIndex++]="Nur 802.11g";
textArray8[iIndex++]="Gemischter 802.11bgn";
textArray8[iIndex++]="Gemischter 802.11bg";
textArray8[iIndex++]="Daten�bertragungsraten :";
textArray8[iIndex++]="Rx-Signalintensit�t :";
textArray8[iIndex++]="Verbindungqualit�t :";
textArray8[iIndex++]="Authentifizierung :";
textArray8[iIndex++]="Shared Key";
textArray8[iIndex++]="WPA-PSK";
textArray8[iIndex++]="WPA2-PSK";
textArray8[iIndex++]="Open System";
textArray8[iIndex++]="Verschl�sselung :";
textArray8[iIndex++]="WEP 64-bit";
textArray8[iIndex++]="WEP 128-bit";
textArray8[iIndex++]="keiner";
textArray8[iIndex++]="deaktiviert";
iIndex = 0;

// out of Deutsch
