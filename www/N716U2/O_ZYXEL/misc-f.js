

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Francais
tabArray=['Etat','Param�trage','Divers','Red�marrer','Pr�r�glages d�usine par d�faut','Mise � niveau du microprogramme'];

	//upgrade.htm
headArray[iIndex++] ="<br>Cette page vous permet de mettre � niveau le microprogramme du serveur d'impression.<br>Remarque : assurez-vous que le microprogramme est correct avec de continuer. Si vous ne savez pas quel microprogramme vous devez utiliser, contactez votre vendeur pour le support technique.";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "Cliquez sur param�tres par d�faut d'usine puis sur OK pour recharger tous les param�tres par d�faut dans le serveur d'impression. Avertissement ! Tous les param�tres actuels seront effac�s.";
textArray1[iIndex++] = "Cliquez sur Mise � niveau du microprogramme pour parcourir le  r�pertoire de votre microprogramme et recharger le serveur d'impression avec le nouveau microprogramme.";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;OK&nbsp;&nbsp;" onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="Mise � niveau du microprogramme";
textArray2[iIndex++]="S�lectionnez R�pertoire du microprogramme et Fichier :";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="Mise � niveau du microprogramme" onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="Cette page vous permet de red�marrer le serveur d'impression.<br>";
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>Red�marrer le serveur d'impression</B></FONT><br><br>Voulez-vous enregistrer les param�tres et red�marrer le serveur d'impression maintenant?<br><br>";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;OK&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

	//restart.htm
textArray4[iIndex++] = "Red�marrage�";
textArray4[iIndex++] = "Patientez pendant le red�marrage du serveur d'impression.";
iIndex = 0;
	//urestart.htm
textArray5[iIndex++] = "Mise � niveau termin�e!";
textArray5[iIndex++] = "Apr�s avoir mis � niveau le microprogramme, le serveur d'impression red�marrera automatiquement, patientez quelques instants.";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "Chargement des param�tres d'usine par d�faut�";
textArray6[iIndex++] = "Apr�s avoir charg� les param�tres par d�faut, le serveur d'impression red�marrera automatiquement.<br><br>Vous serez redirig� vers la page Etat quand le serveur d'impression aura red�marr�.";
iIndex = 0;
// out of Francais
