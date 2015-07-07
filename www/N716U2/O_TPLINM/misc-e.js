

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : English
tabArray=['<IMG SRC=images/MPS110U-SC.gif>','Status','Setup','Misc','Restart','Factory Default','Firmware Upgrade','Restart'];

	//upgrade.htm
headArray[iIndex++] ="<br>This page allows you to upgrade the firmware of the print server.<br>Note: please make sure the firmware is correct before you proceed. If you do not know which firmware file you should use, please contact your local dealer for technical support.";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "Click <b>Factory Default</b> then <b>OK</b> to reload all default settings in the print server. Warning! All current settings will be erased.";
textArray1[iIndex++] = "Click <b>Firmware Upgrade</b> to browse to your firmware directory and reload the print server with new firmware.";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;OK&nbsp;&nbsp;" onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="Firmware Upgrade";
textArray2[iIndex++]="Select Firmware Directory and File:";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="Firmware Upgrade" onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="This page allows you to restart the print server.<br>";
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>Restart The Print Server</B></FONT><br><br>Do you want to save settings and restart the print server now ?<br><br>";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;OK&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

	//restart.htm
textArray4[iIndex++] = "Restarting ...";
textArray4[iIndex++] = "Please wait while the print server restarts.";
iIndex = 0;
	//urestart.htm
textArray5[iIndex++] = "Upgrade completed successfully!";
textArray5[iIndex++] = "After upgrading the firmware, the print server will automatically restart, please wait a few moments.";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "Loading Factory Defaults ...";
textArray6[iIndex++] = "After loading the default settings, the print server will automatically restart.<BR><BR>You will be redirected to the Status page when the print server has been restarted.";
iIndex = 0;
// out of English
