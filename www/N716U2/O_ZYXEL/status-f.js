//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

tabArray=['Etat','Param�trage','Divers','Red�marrer','Syst�me','Imprimante','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
//Language : Francais

//system.htm
headArray[iIndex++] = "<BR>Cette page affiche les informations syst�me g�n�rales du serveur d'impression.<BR>";
//printer.htm
headArray[iIndex++] = "<BR>Cette page affiche les informations de l'imprimante qui est actuellement connect�e au serveur d'impression.<BR>Remarque : Si votre imprimante ne prend pas en charge la fonction bi-directionnelle, certaines informations peuvent ne pas s'afficher correctement.";
//tcpip.htm
headArray[iIndex++] = "<BR>Cette page affiche les param�tres TCP/IP actuels du serveur d'impression.<BR>";
//netware.htm
headArray[iIndex++] = "<BR>Cette page affiche les param�tres NetWare actuels du serveur d'impression.<BR>";
//apple.htm
headArray[iIndex++] = "<BR>Cette page affiche les param�tres AppleTalk actuels du serveur d'impression.<BR>";
//snmp.htm
headArray[iIndex++] = "<BR>Cette page affiche les param�tres SNMP actuels du serveur d'impression.<BR>";
//Smb.htm
headArray[iIndex++] = "<BR>Cette page affiche les param�tres de partage d'imprimante pour les r�seaux Microsoft Windows.<BR>";
iIndex = 0;

// SYSTEM.HTM
textArray0[iIndex++]="Informations du syst�me";
textArray0[iIndex++]="Nom du serveur d'impression :";
textArray0[iIndex++]="Contact du Syst�me :";
textArray0[iIndex++]="Emplacement du Syst�me :";
textArray0[iIndex++]="Temps d'activit� du syst�me :";
textArray0[iIndex++]="Version du microprogramme :";
textArray0[iIndex++]="Adresse MAC :";
iIndex = 0;

// PRINTER.HTM
textArray1[iIndex++]="Informations de l'imprimante";
textArray1[iIndex++]="Fabricant";
textArray1[iIndex++]="Num�ro de mod�le";
textArray1[iIndex++]="Langues d'impression prises en charge";
textArray1[iIndex++]="�tat actuel";
iIndex = 0;

// NETWARE.HTM
textArray2[iIndex++]="Param�tres g�n�raux";
textArray2[iIndex++]="Nom du serveur d'impression :";
textArray2[iIndex++]="Temps d'interrogation :";
textArray2[iIndex++]="Param�tres NDS NetWare";
textArray2[iIndex++]="Utiliser le mode NDS :";
textArray2[iIndex++]="Nom de l'arborescence NDS :";
textArray2[iIndex++]="Nom du contexte NDS :";
textArray2[iIndex++]="�tat actuel :";
textArray2[iIndex++]="Param�tres de NetWare Bindery";
textArray2[iIndex++]="Utiliser le mode Bindery :";
textArray2[iIndex++]="Nom du serveur de fichiers :";
textArray2[iIndex++]="�tat actuel :";
iIndex = 0;

// TCPIP.HTM
textArray3[iIndex++]="Param�tres TCP/IP";
textArray3[iIndex++]="Utiliser DHCP/BOOTP :";
textArray3[iIndex++]="Adresse IP :";
textArray3[iIndex++]="Masque de sous-r�seau :";
textArray3[iIndex++]="Passerelle :";
iIndex = 0;

// APPLE.HTM
textArray4[iIndex++]="Param�tres AppleTalk";
textArray4[iIndex++]="AppleTalk:";
textArray4[iIndex++]="Informations sur l'imprimante";
textArray4[iIndex++]="Nom du port :";
textArray4[iIndex++]="Type d'imprimante :";
textArray4[iIndex++]="Format des donn�es :";
iIndex = 0;

// SNMP.HTM
textArray5[iIndex++]="Param�tres de communaut� SNMP";
textArray5[iIndex++]="Communaut� 1 SNMP :";
textArray5[iIndex++]="Communaut� 2 SNMP :";
textArray5[iIndex++]="Param�tres d'interruption SNMP";
textArray5[iIndex++]="Envoyer les interruptions SNMP :";
textArray5[iIndex++]="Utiliser des interruptions d'authentification :";
textArray5[iIndex++]="Adresse 1 d'interruption :";
textArray5[iIndex++]="Adresse 2 d'interruption :";
iIndex = 0;



// SMB.HTM
textArray7[iIndex++]="Groupe de travail";
textArray7[iIndex++]="Nom :";
textArray7[iIndex++]="Nom partag�";
textArray7[iIndex++]="Imprimante :";


// out of Francais
