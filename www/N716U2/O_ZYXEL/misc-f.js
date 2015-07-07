

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Francais
tabArray=['Etat','Paramétrage','Divers','Redémarrer','Préréglages d’usine par défaut','Mise à niveau du microprogramme'];

	//upgrade.htm
headArray[iIndex++] ="<br>Cette page vous permet de mettre à niveau le microprogramme du serveur d'impression.<br>Remarque : assurez-vous que le microprogramme est correct avec de continuer. Si vous ne savez pas quel microprogramme vous devez utiliser, contactez votre vendeur pour le support technique.";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "Cliquez sur paramètres par défaut d'usine puis sur OK pour recharger tous les paramètres par défaut dans le serveur d'impression. Avertissement ! Tous les paramètres actuels seront effacés.";
textArray1[iIndex++] = "Cliquez sur Mise à niveau du microprogramme pour parcourir le  répertoire de votre microprogramme et recharger le serveur d'impression avec le nouveau microprogramme.";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;OK&nbsp;&nbsp;" onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="Mise à niveau du microprogramme";
textArray2[iIndex++]="Sélectionnez Répertoire du microprogramme et Fichier :";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="Mise à niveau du microprogramme" onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="Cette page vous permet de redémarrer le serveur d'impression.<br>";
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>Redémarrer le serveur d'impression</B></FONT><br><br>Voulez-vous enregistrer les paramètres et redémarrer le serveur d'impression maintenant?<br><br>";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;OK&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

	//restart.htm
textArray4[iIndex++] = "Redémarrage…";
textArray4[iIndex++] = "Patientez pendant le redémarrage du serveur d'impression.";
iIndex = 0;
	//urestart.htm
textArray5[iIndex++] = "Mise à niveau terminée!";
textArray5[iIndex++] = "Après avoir mis à niveau le microprogramme, le serveur d'impression redémarrera automatiquement, patientez quelques instants.";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "Chargement des paramètres d'usine par défaut…";
textArray6[iIndex++] = "Après avoir chargé les paramètres par défaut, le serveur d'impression redémarrera automatiquement.<br><br>Vous serez redirigé vers la page Etat quand le serveur d'impression aura redémarré.";
iIndex = 0;
// out of Francais
