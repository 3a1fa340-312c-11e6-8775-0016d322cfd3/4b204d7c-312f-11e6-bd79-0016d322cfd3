//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

tabArray=['Stato','Impostazione','Varie','Riavvio','Sistema','Stampante','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
//Language : Italiano

//system.htm
headArray[iIndex++] = "<BR>Questa pagina consente di visualizzare le informazioni di sistema generali del server di stampa.<BR>";
//printer.htm
headArray[iIndex++] = "<BR>Questa pagina consente di visualizzare le informazioni della stampante attualmente collegata al server di stampa.<BR>Nota: Se la stampante non supporta la funzione bidirezionale, alcune informazioni potrebbero non essere visualizzate correttamente.";
//tcpip.htm
headArray[iIndex++] = "<BR>Questa pagina consente di visualizzare le impostazioni TCP/IP correnti del server di stampa.<BR>";
//netware.htm
headArray[iIndex++] = "<BR>Questa pagina consente di visualizzare le impostazioni NetWare correnti del server di stampa.<BR>";
//apple.htm
headArray[iIndex++] = "<BR>Questa pagina consente di visualizzare le impostazioni AppleTalk correnti del server di stampa.<BR>";
//snmp.htm
headArray[iIndex++] = "<BR>Questa pagina consente di visualizzare le impostazioni SNMP correnti del server di stampa.<BR>";
//Smb.htm
headArray[iIndex++] = "<BR>Questa pagina consente di visualizzare le impostazioni di condivisione della stampante per le reti di Microsoft Windows.<BR>";
iIndex = 0;

// SYSTEM.HTM
textArray0[iIndex++]="Informazioni di sistema";
textArray0[iIndex++]="Nome server di stampa:";
textArray0[iIndex++]="Contatto sistema:";
textArray0[iIndex++]="Posizione sistema:";
textArray0[iIndex++]="Tempo attività sistema:";
textArray0[iIndex++]="Versione firmware:";
textArray0[iIndex++]="Indirizzo MAC:";
iIndex = 0;

// PRINTER.HTM
textArray1[iIndex++]="Informazioni stampante";
textArray1[iIndex++]="Produttore";
textArray1[iIndex++]="Numero modello";
textArray1[iIndex++]="Lingue supportate per la stampa";
textArray1[iIndex++]="Stato corrente";
iIndex = 0;

// NETWARE.HTM
textArray2[iIndex++]="Impostazioni generali";
textArray2[iIndex++]="Nome server di stampa:";
textArray2[iIndex++]="Intervallo di tempo:";
textArray2[iIndex++]="Impostazioni NDS NetWare";
textArray2[iIndex++]="Usa modalità NDS:";
textArray2[iIndex++]="Nome dell’albero NDS:";
textArray2[iIndex++]="Nome del contesto NDS:";
textArray2[iIndex++]="Stato corrente:";
textArray2[iIndex++]="Impostazioni rilegatura NetWare";
textArray2[iIndex++]="Usa modalità rilegatura:";
textArray2[iIndex++]="Nome server file:";
textArray2[iIndex++]="Stato corrente:";
iIndex = 0;

// TCPIP.HTM
textArray3[iIndex++]="Impostazioni TCP/IP";
textArray3[iIndex++]="Usa DHCP/BOOTP:";
textArray3[iIndex++]="Indirizzo IP:";
textArray3[iIndex++]="Subnet Mask:";
textArray3[iIndex++]="Gateway:";
iIndex = 0;

// APPLE.HTM
textArray4[iIndex++]="Impostazioni AppleTalk";
textArray4[iIndex++]="AppleTalk:";
textArray4[iIndex++]="Informazioni sulla stampante";
textArray4[iIndex++]="Nome porta:";
textArray4[iIndex++]="Tipo stampante:";
textArray4[iIndex++]="Formato data:";
iIndex = 0;

// SNMP.HTM
textArray5[iIndex++]="Impostazioni comunità SNMP";
textArray5[iIndex++]="Comunità SNMP 1:";
textArray5[iIndex++]="Comunità SNMP 2:";
textArray5[iIndex++]="Impostazioni trap SNMP";
textArray5[iIndex++]="Invia trap SNMP:";
textArray5[iIndex++]="Usa trap di autenticazione:";
textArray5[iIndex++]="Indirizzo trap 1:";
textArray5[iIndex++]="Indirizzo trap 2:";
iIndex = 0;



// SMB.HTM
textArray7[iIndex++]="Gruppo di lavoro";
textArray7[iIndex++]="Nome:";
textArray7[iIndex++]="Nome condiviso";
textArray7[iIndex++]="Stampante:";


// out of Italiano
