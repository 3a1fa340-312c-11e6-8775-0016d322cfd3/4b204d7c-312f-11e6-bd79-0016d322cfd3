

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Deutsch
tabArray=['Druckservers','Status','Installation','Sonstiges','Neustart','Standardwerte','Firmwareaktualisierung','Neustart'];

	//upgrade.htm
headArray[iIndex++] ="<br>Mit Hilfe dieser Seite können Sie die Firmware des Druckservers aktualisieren.<br>Hinweis: Bevor Sie fortfahren, sollten Sie sicherstellen, dass die richtige Firmware gewählt wurde. Wenn Sie sich nicht sicher sind, welche Firmwaredatei verwendet werden muss, wenden Sie sich an einen Fachhändler.";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "Klicken Sie auf <b>Standardwerte</b> und dann auf <b>OK</b>, um alle Standardeinstellungen in den Druckserver zu laden. Warnung! Alle aktuellen Einstellungen werden gelöscht.";
textArray1[iIndex++] = "Standardeinstellungen laden, ohne TCP/IP-Einstellungen. (Empfohlen)";
textArray1[iIndex++] = "Sie können weiterhin Schließen Sie den Druckserver mit der aktuellen IP-Adresse nach dem Neustart.";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;OK&nbsp;&nbsp;" onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="Firmwareaktualisierung";
textArray2[iIndex++]="Firmwareverzeichnis und Datei wählen:";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="Firmwareaktualisierung" onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="Auf dieser Seite können Sie den Druckserver neu starten.<br>";
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>Druckserver neu starten</B></FONT><br><br>Sollen jetzt die Einstellungen gespeichert und der Druckserver neu gestartet werden?<br><br>";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;OK&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

	//restart.htm
textArray4[iIndex++] = "Neustart...";
textArray4[iIndex++] = "Warten Sie, während der Druckserver neu gestartet wird.";
iIndex = 0;
	//urestart.htm
textArray5[iIndex++] = "Aktualisierung erfolgreich abgeschlossen!";
textArray5[iIndex++] = "Nach dem Aktualisieren der Firmware wird der Druckserver automatisch neu gestartet. Bitte warten Sie einen Moment.";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "Lade Standardwerte...";
textArray6[iIndex++] = "Nach dem Laden der Standardwerte wird der Druckserver automatisch neu gestartet.<br><br>Wenn der Druckserver neu gestartet wurde, werden Sie zur Statusseite umgeleitet.";
iIndex = 0;
// out of Deutsch
