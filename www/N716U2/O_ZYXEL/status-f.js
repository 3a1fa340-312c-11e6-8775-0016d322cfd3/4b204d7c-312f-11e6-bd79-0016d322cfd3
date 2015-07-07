//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

tabArray=['Etat','Paramétrage','Divers','Redémarrer','Système','Imprimante','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
//Language : Francais

//system.htm
headArray[iIndex++] = "<BR>Cette page affiche les informations système générales du serveur d'impression.<BR>";
//printer.htm
headArray[iIndex++] = "<BR>Cette page affiche les informations de l'imprimante qui est actuellement connectée au serveur d'impression.<BR>Remarque : Si votre imprimante ne prend pas en charge la fonction bi-directionnelle, certaines informations peuvent ne pas s'afficher correctement.";
//tcpip.htm
headArray[iIndex++] = "<BR>Cette page affiche les paramètres TCP/IP actuels du serveur d'impression.<BR>";
//netware.htm
headArray[iIndex++] = "<BR>Cette page affiche les paramètres NetWare actuels du serveur d'impression.<BR>";
//apple.htm
headArray[iIndex++] = "<BR>Cette page affiche les paramètres AppleTalk actuels du serveur d'impression.<BR>";
//snmp.htm
headArray[iIndex++] = "<BR>Cette page affiche les paramètres SNMP actuels du serveur d'impression.<BR>";
//Smb.htm
headArray[iIndex++] = "<BR>Cette page affiche les paramètres de partage d'imprimante pour les réseaux Microsoft Windows.<BR>";
iIndex = 0;

// SYSTEM.HTM
textArray0[iIndex++]="Informations du système";
textArray0[iIndex++]="Nom du serveur d'impression :";
textArray0[iIndex++]="Contact du Système :";
textArray0[iIndex++]="Emplacement du Système :";
textArray0[iIndex++]="Temps d'activité du système :";
textArray0[iIndex++]="Version du microprogramme :";
textArray0[iIndex++]="Adresse MAC :";
iIndex = 0;

// PRINTER.HTM
textArray1[iIndex++]="Informations de l'imprimante";
textArray1[iIndex++]="Fabricant";
textArray1[iIndex++]="Numéro de modèle";
textArray1[iIndex++]="Langues d'impression prises en charge";
textArray1[iIndex++]="État actuel";
iIndex = 0;

// NETWARE.HTM
textArray2[iIndex++]="Paramètres généraux";
textArray2[iIndex++]="Nom du serveur d'impression :";
textArray2[iIndex++]="Temps d'interrogation :";
textArray2[iIndex++]="Paramètres NDS NetWare";
textArray2[iIndex++]="Utiliser le mode NDS :";
textArray2[iIndex++]="Nom de l'arborescence NDS :";
textArray2[iIndex++]="Nom du contexte NDS :";
textArray2[iIndex++]="État actuel :";
textArray2[iIndex++]="Paramètres de NetWare Bindery";
textArray2[iIndex++]="Utiliser le mode Bindery :";
textArray2[iIndex++]="Nom du serveur de fichiers :";
textArray2[iIndex++]="État actuel :";
iIndex = 0;

// TCPIP.HTM
textArray3[iIndex++]="Paramètres TCP/IP";
textArray3[iIndex++]="Utiliser DHCP/BOOTP :";
textArray3[iIndex++]="Adresse IP :";
textArray3[iIndex++]="Masque de sous-réseau :";
textArray3[iIndex++]="Passerelle :";
iIndex = 0;

// APPLE.HTM
textArray4[iIndex++]="Paramètres AppleTalk";
textArray4[iIndex++]="AppleTalk:";
textArray4[iIndex++]="Informations sur l'imprimante";
textArray4[iIndex++]="Nom du port :";
textArray4[iIndex++]="Type d'imprimante :";
textArray4[iIndex++]="Format des données :";
iIndex = 0;

// SNMP.HTM
textArray5[iIndex++]="Paramètres de communauté SNMP";
textArray5[iIndex++]="Communauté 1 SNMP :";
textArray5[iIndex++]="Communauté 2 SNMP :";
textArray5[iIndex++]="Paramètres d'interruption SNMP";
textArray5[iIndex++]="Envoyer les interruptions SNMP :";
textArray5[iIndex++]="Utiliser des interruptions d'authentification :";
textArray5[iIndex++]="Adresse 1 d'interruption :";
textArray5[iIndex++]="Adresse 2 d'interruption :";
iIndex = 0;



// SMB.HTM
textArray7[iIndex++]="Groupe de travail";
textArray7[iIndex++]="Nom :";
textArray7[iIndex++]="Nom partagé";
textArray7[iIndex++]="Imprimante :";


// out of Francais
