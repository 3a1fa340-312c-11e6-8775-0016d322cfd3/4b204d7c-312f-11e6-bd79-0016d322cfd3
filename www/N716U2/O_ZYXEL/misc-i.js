

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Italiano
tabArray=['Stato','Impostazione','Varie','Riavvio','Impostazioni predefinite','Aggiornamento firmware'];

	//upgrade.htm
headArray[iIndex++] ="<br>Questa pagina consente di aggiornare il firmware del server di stampa.<br>Nota: verificare che il firmware sia corretto prima di continuare. Se non si conosce il firmware da utilizzare, contattare il proprio rivenditore per assistenza tecnica.";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "Fare clic su <b>Impostazione predefinita</b> quindi fare clic su <b>OK</b> per ricaricare tutte le impostazioni predefinite nel server di stampa. Attenzione Tutte le impostazioni correnti verranno cancellate.";
textArray1[iIndex++] = "Fare clic su <b>Aggiornamento firmware</b> per selezionare la directory del firmware e aggiornare il server di stampa con il nuovo firmware.";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;OK&nbsp;&nbsp;" onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="Aggiornamento firmware";
textArray2[iIndex++]="Selezionare il file e la directory del firmware:";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="Aggiornamento firmware" onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="Questa pagina consente di riavviare il server di stampa.<br>";
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>Riavvia il server di stampa</B></FONT><br><br>Salvare le impostazioni e riavviare il server di stampa adesso?<br><br>";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;OK&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

	//restart.htm
textArray4[iIndex++] = "Riavvio in corso...";
textArray4[iIndex++] = "Attendere durante il riavvio del server di stampa.";
iIndex = 0;
	//urestart.htm
textArray5[iIndex++] = "Aggiornamento eseguito correttamente.";
textArray5[iIndex++] = "Dopo l’aggiornamento del firmware, il server di stampa verrà avviato automaticamente, attendere alcuni secondi.";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "Caricamento delle impostazioni predefinite in corso...";
textArray6[iIndex++] = "Dopo il caricamento delle impostazioni predefinite, il server di stampa verrà riavviato automaticamente.<br><br>Una volta riavviato il server di stampa, verrà visualizzata la pagina Stato.";
iIndex = 0;
// out of Italiano
