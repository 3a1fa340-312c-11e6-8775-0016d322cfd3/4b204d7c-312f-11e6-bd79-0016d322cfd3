
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Francais
tabArray=['Etat','Param�trage','Divers','Red�marrer','Syst�me','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>Cette page de param�trage vous permet de configurer les param�tres du syst�me g�n�ral du serveur d'impression.<br>";
//ctcpip.htm
headArray[iIndex++] = "<BR>Cette page de param�trage vous permet de configurer les param�tres TCP/IP du serveur d'impression.";
//cnetware.htm
headArray[iIndex++] = "<BR>Cette page de param�trage vous permet de configurer la fonction NetWare du serveur d'impression.";
//capple.htm 
headArray[iIndex++] ="<BR>Cette page de param�trage vous permet de configurer les param�tres AppleTalk du serveur d'impression.<br>";
//csnmp.htm
headArray[iIndex++] ="<BR>Cette page de param�trage vous permet de configurer les param�tres SNMP du serveur d'impression.";
//csmp.htm
headArray[iIndex++] ="<BR>Cette page affiche les param�tres de partage d'imprimante pour les r�seaux Microsoft Windows.";
iIndex = 0;

// CSYSTEM.HTM
textArray0[iIndex++]="Param�tres syst�me";
textArray0[iIndex++]="Nom du serveur d'impression :";
textArray0[iIndex++]="Contact du Syst�me :";
textArray0[iIndex++]="Emplacement du Syst�me :";
textArray0[iIndex++]="Mot de passe administrateur";
textArray0[iIndex++]="Mot de passe :";
textArray0[iIndex++]="Retapez le mot de passe :";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="Enregistrer & Red�marrer" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
// CPRINTER.HTM
textArray1[iIndex++]="Port parall�le � Param�tres bi-directionnels";
textArray1[iIndex++]="Port 1 d'imprimante :";
textArray1[iIndex++]='<input type="button" value="Enregistrer & Red�marrer" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
// CTCPIP.HTM
textArray2[iIndex++]="Param�tres TCP/IP";
textArray2[iIndex++]="Obtenez automatiquement les param�tres TCP/IP (utilisez DHCP/BOOTP)";
textArray2[iIndex++]="Utilisez les param�tres TCP/IP suivants";
textArray2[iIndex++]="Adresse IP :";
textArray2[iIndex++]="Masque de sous-r�seau :";
textArray2[iIndex++]="Routeur par d�faut :";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="Enregistrer & Red�marrer" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="Param�tres AppleTalk";
textArray3[iIndex++]="Zone AppleTalk :";
textArray3[iIndex++]="Nom du port :";
textArray3[iIndex++]="Configuration d'imprimante";
textArray3[iIndex++]="Type :";
textArray3[iIndex++]="Protocole binaire :";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="Enregistrer & Red�marrer" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
// CSNMP.HTM
textArray4[iIndex++]="Param�tres de communaut� SNMP";
textArray4[iIndex++]="Prend en charge HP WebJetAdmin :";
textArray4[iIndex++]="Nom 1 de communaut� SNMP :";
textArray4[iIndex++]="Privil�ge :";
textArray4[iIndex++]="Nom 2 de communaut� SNMP :";
textArray4[iIndex++]="Privil�ge :";
textArray4[iIndex++]="Param�tres d'interruption SNMP";
textArray4[iIndex++]="Envoyer les interruptions SNMP :";
textArray4[iIndex++]="Utiliser des interruptions d'authentification :";
textArray4[iIndex++]="Adresse 1 d'interruption :";
textArray4[iIndex++]="Adresse 2 d'interruption :";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="Enregistrer & Red�marrer" onClick="return SaveSetting(';
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
textArray7[iIndex++]="ERREUR";
textArray7[iIndex++]="Revenir �";
iIndex = 0;

// CNETWARE.HTM
textArray8[iIndex++]="Param�tres g�n�raux";
textArray8[iIndex++]="Nom du serveur d'impression :";
textArray8[iIndex++]="Temps d'interrogation :";
textArray8[iIndex++]="&nbsp;secondes (min: 3, max : 29 secondes)";
textArray8[iIndex++]="Mot de passe d'ouverture de session :";
textArray8[iIndex++]="Param�tres NDS NetWare";
textArray8[iIndex++]="Utiliser le mode NDS :";
textArray8[iIndex++]="Nom de l'arborescence NDS :";
textArray8[iIndex++]="Nom du contexte NDS :";
textArray8[iIndex++]="Param�tres de NetWare Bindery";
textArray8[iIndex++]="Utiliser le mode Bindery :";
textArray8[iIndex++]="Nom du serveur de fichiers :";
textArray8[iIndex++]='<input type="button" value="Enregistrer & Red�marrer" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.HTM
textArray9[iIndex++]="Groupe de travail";
textArray9[iIndex++]="Nom :";
textArray9[iIndex++]="Nom partag�";
textArray9[iIndex++]="Imprimante :";
textArray9[iIndex++]='<input type="button" value="Enregistrer & Red�marrer" onClick="return SaveSetting(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;
// out of Francais
