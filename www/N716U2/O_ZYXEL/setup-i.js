
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Italiano
tabArray=['Stato','Impostazione','Varie','Riavvio','Sistema','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>Questa pagina di impostazione consente di configurare le impostazioni di sistema generali del server di stampa.<br>";
//ctcpip.htm
headArray[iIndex++] = "<BR>Questa pagina di impostazione consente di configurare le impostazioni TCP/IP del server di stampa.";
//cnetware.htm
headArray[iIndex++] = "<BR>Questa pagina di impostazione consente di configurare la funzione NetWare del server di stampa.";
//capple.htm 
headArray[iIndex++] ="<BR>Questa pagina di impostazione consente di configurare le impostazioni AppleTalk del server di stampa.<br>";
//csnmp.htm
headArray[iIndex++] ="<BR>Questa pagina di impostazione consente di configurare le impostazioni SNMP del server di stampa.";
//csmp.htm
headArray[iIndex++] ="<BR>Questa pagina consente di visualizzare le impostazioni di condivisione della stampante per le reti di Microsoft Windows.";
iIndex = 0;

// CSYSTEM.HTM
textArray0[iIndex++]="Impostazioni sistema";
textArray0[iIndex++]="Nome server di stampa:";
textArray0[iIndex++]="Contatto sistema:";
textArray0[iIndex++]="Posizione sistema:";
textArray0[iIndex++]="Password amministratore";
textArray0[iIndex++]="Password:";
textArray0[iIndex++]="Reimmettere password:";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="Salva e riavvia" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
// CPRINTER.HTM
textArray1[iIndex++]="Porta parallela – Impostazioni bidirezionali";
textArray1[iIndex++]="Porta stampante 1:";
textArray1[iIndex++]='<input type="button" value="Salva e riavvia" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
// CTCPIP.HTM
textArray2[iIndex++]="Impostazioni TCP/IP";
textArray2[iIndex++]="Ottieni impostazioni TCP/IP automaticamente (usa DHCP/BOOTP)";
textArray2[iIndex++]="Usa le impostazioni TCP/IP seguenti";
textArray2[iIndex++]="Indirizzo IP:";
textArray2[iIndex++]="Subnet Mask:";
textArray2[iIndex++]="Router predefinito:";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="Salva e riavvia" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="Impostazioni AppleTalk";
textArray3[iIndex++]="Area AppleTalk:";
textArray3[iIndex++]="Nome porta:";
textArray3[iIndex++]="Configurazione stampante";
textArray3[iIndex++]="Tipo:";
textArray3[iIndex++]="Protocollo binario:";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="Salva e riavvia" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
// CSNMP.HTM
textArray4[iIndex++]="Impostazioni comunità SNMP";
textArray4[iIndex++]="Supporto HP WebJetAdmin:";
textArray4[iIndex++]="Nome comunità SNMP 1:";
textArray4[iIndex++]="Privilegio:";
textArray4[iIndex++]="Nome comunità SNMP 2:";
textArray4[iIndex++]="Privilegio:";
textArray4[iIndex++]="Impostazioni trap SNMP";
textArray4[iIndex++]="Invia trap SNMP:";
textArray4[iIndex++]="Usa trap di autenticazione:";
textArray4[iIndex++]="Indirizzo trap 1:";
textArray4[iIndex++]="Indirizzo trap 2:";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="Salva e riavvia" onClick="return SaveSetting(';
// Begin don't translate
textArray4[iIndex++]="'RESTART.HTM');";
textArray4[iIndex++]='">';
iIndex = 0;
// End don't translate
//keyhelp.htm
//textArray5[iIndex++]="<b>WEP Key Format</b>";
//textArray5[iIndex++]="An alphanumeric character is 'a' through 'z', 'A' through 'Z', and '0' through '9'.";
//textArray5[iIndex++]="A hexadecimal digit is '0' through '9' and 'A' through 'F'.";
//textArray5[iIndex++]="Depending on the key format you select:";
//textArray5[iIndex++]="For 64-bit (sometimes called 40-bit) WEP encryption, enter the key which contains 5 alphanumeric characters or 10 hexadecimal digits. For example: AbZ12 (alphanumeric format) or ABCDEF1234 (hexadecimal format).";
//textArray5[iIndex++]="For 128-bit WEP encryption, enter the key which contains 13 alphanumeric characters or 26 hexadecimal digits.";
// Translate                                  only "Close" is to be translated
textArray5[iIndex++]='<INPUT TYPE=button VALUE=" Close " onClick="window.close()">';
iIndex = 0;
//browser.htm
//textArray6[iIndex++]="SSID";
//textArray6[iIndex++]="AP's MAC Address or BSSID";
//textArray6[iIndex++]="Channel";
//textArray6[iIndex++]="Type";
//textArray6[iIndex++]="WEP/WPA-PSK";
//textArray6[iIndex++]="Signal Strength";
// Translate                                  only "Rescan" is to be translated
//textArray6[iIndex++]='<INPUT TYPE=submit VALUE="Rescan">';
// Translate                                  only "Close" is to be translated
//textArray6[iIndex++]='<INPUT TYPE=button VALUE=" Close " onClick="window.close()">';
//iIndex = 0;
// End don't translate
// ERROR.htm
textArray7[iIndex++]="ERRORE";
textArray7[iIndex++]="Torna indietro";
iIndex = 0;

// CNETWARE.HTM
textArray8[iIndex++]="Impostazioni generali";
textArray8[iIndex++]="Nome server di stampa:";
textArray8[iIndex++]="Intervallo di tempo:";
textArray8[iIndex++]="&nbsp;secondi (min: 3, max: 29 secondi)";
textArray8[iIndex++]="Password accesso:";
textArray8[iIndex++]="Impostazioni NDS NetWare";
textArray8[iIndex++]="Usa modalità NDS:";
textArray8[iIndex++]="Nome dell’albero NDS:";
textArray8[iIndex++]="Nome del contesto NDS:";
textArray8[iIndex++]="Impostazioni rilegatura NetWare";
textArray8[iIndex++]="Usa modalità rilegatura:";
textArray8[iIndex++]="Nome server file:";
textArray8[iIndex++]='<input type="button" value="Salva e riavvia" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.HTM
textArray9[iIndex++]="Gruppo di lavoro";
textArray9[iIndex++]="Nome:";
textArray9[iIndex++]="Nome condiviso";
textArray9[iIndex++]="Stampante:";
textArray9[iIndex++]='<input type="button" value="Salva e riavvia" onClick="return SaveSetting(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;
// out of Italiano
