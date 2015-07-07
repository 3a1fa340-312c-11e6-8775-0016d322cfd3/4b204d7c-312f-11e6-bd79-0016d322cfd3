//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

tabArray=['Status','Installation','Sonstiges','Neustart','System','Drucker','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
//Language : Deutsch

//system.htm
headArray[iIndex++] = "<BR>Auf dieser Seite werden die allgemeinen Systemdaten des Druckservers angezeigt.<BR>";
//printer.htm
headArray[iIndex++] = "<BR>Auf dieser Seite werden die Daten des Druckers angezeigt, der gegenwärtig an den Druckserver angeschlossen ist.<BR>Hinweis: Wenn Ihr Drucker nicht bidirektional funktioniert, ist es möglich, dass einige Daten nicht richtig wiedergegeben werden.";
//tcpip.htm
headArray[iIndex++] = "<BR>Auf dieser Seite werden die aktuellen TCP/IP-Einstellungen des Druckservers angezeigt.<BR>";
//netware.htm
headArray[iIndex++] = "<BR>Auf dieser Seite werden die aktuellen NetWare-Einstellungen des Druckservers angezeigt.<BR>";
//apple.htm
headArray[iIndex++] = "<BR>Auf dieser Seite werden die aktuellen AppleTalk-Einstellungen des Druckservers angezeigt.<BR>";
//snmp.htm
headArray[iIndex++] = "<BR>Auf dieser Seite werden die aktuellen SNMP-Einstellungen des Druckservers angezeigt.<BR>";
//Smb.htm
headArray[iIndex++] = "<BR>Auf dieser Seite werden die Einstellungen für die gemeinsame Druckerverwendung bei Microsoft Windows-Netzwerken angezeigt.<BR>";
iIndex = 0;

// SYSTEM.HTM
textArray0[iIndex++]="Systemdaten";
textArray0[iIndex++]="Name des Druckservers:";
textArray0[iIndex++]="Systemkontakt:";
textArray0[iIndex++]="Systemposition:";
textArray0[iIndex++]="Systembetriebszeit:";
textArray0[iIndex++]="Firmwareversion:";
textArray0[iIndex++]="MAC-Adresse:";
iIndex = 0;

// PRINTER.HTM
textArray1[iIndex++]="Daten Drucker";
textArray1[iIndex++]="Hersteller";
textArray1[iIndex++]="Modellnummer";
textArray1[iIndex++]="Unterstützte Drucksprache";
textArray1[iIndex++]="Aktueller Status";
iIndex = 0;

// NETWARE.HTM
textArray2[iIndex++]="Allgemeine Einstellungen";
textArray2[iIndex++]="Name des Druckservers:";
textArray2[iIndex++]="Polling-Zeit:";
textArray2[iIndex++]="NDS-Einstellungen von NetWare";
textArray2[iIndex++]="NDS-Modus verwenden:";
textArray2[iIndex++]="Name des NDS-Baums:";
textArray2[iIndex++]="Name des NDS-Kontexts:";
textArray2[iIndex++]="Aktueller Status:";
textArray2[iIndex++]="NetWare-Bindery-Einstellungen";
textArray2[iIndex++]="Bindery-Modus verwenden:";
textArray2[iIndex++]="Name des Dateiservers:";
textArray2[iIndex++]="Aktueller Status:";
iIndex = 0;

// TCPIP.HTM
textArray3[iIndex++]="TCP/IP-Einstellungen";
textArray3[iIndex++]="DHCP/BOOTP verwenden:";
textArray3[iIndex++]="IP-Adresse:";
textArray3[iIndex++]="Subnet-Mask:";
textArray3[iIndex++]="Gateway:";
iIndex = 0;

// APPLE.HTM
textArray4[iIndex++]="AppleTalk-Einstellungen";
textArray4[iIndex++]="AppleTalk:";
textArray4[iIndex++]="Druckerdaten";
textArray4[iIndex++]="Portname:";
textArray4[iIndex++]="Druckertyp:";
textArray4[iIndex++]="Datenformat:";
iIndex = 0;

// SNMP.HTM
textArray5[iIndex++]="SNMP-Community-Einstellungen";
textArray5[iIndex++]="SNMP-Community 1:";
textArray5[iIndex++]="SNMP-Community 2:";
textArray5[iIndex++]="SNMP-Trap-Einstellungen";
textArray5[iIndex++]="SNMP-Traps senden:";
textArray5[iIndex++]="Authentifizierungs-Traps verwenden:";
textArray5[iIndex++]="Trap-Adresse 1:";
textArray5[iIndex++]="Trap-Adresse 2:";
iIndex = 0;



// SMB.HTM
textArray7[iIndex++]="Workgroup";
textArray7[iIndex++]="Name:";
textArray7[iIndex++]="Gemeinsam genutzter Name";
textArray7[iIndex++]="Drucker:";


// out of Deutsch
