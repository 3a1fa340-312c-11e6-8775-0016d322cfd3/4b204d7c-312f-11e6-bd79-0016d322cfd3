

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : English
tabArray=['<IMG SRC=images/TL-PS110U-EN.jpg>','Status','Setup','Misc','Restart','Factory Default','Firmware Upgrade','Export/Import CFG','Restart'];

	//upgrade.htm
headArray[iIndex++] ="<br>This page allows you to upgrade the firmware of the print server.<br>Note: please make sure the firmware is correct before you proceed. If you do not know which firmware file you should use, please contact your local dealer for technical support.";
	//cfg.htm
headArray[iIndex++] ="<br>This page allows you to export current configuration as file (CFG.bin). Or, import configuration file exported here.";
headArray[iIndex++] ="<br>To export current configuration as file, click Export.";
headArray[iIndex++] ="<br>To import previous configuration from file, click Browse to select a file (CFG.bin) from your local drive and then click Import.";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "Click <b>Factory Default</b> then <b>OK</b> to reload all default settings in the print server.<BR><FONT CLASS=F1 COLOR=#FF3300><B>Warning! All current settings will be erased.</B></FONT>";
textArray1[iIndex++] = "Load default, excluding TCP/IP settings. (Recommended)";
textArray1[iIndex++] = "You can still connect the print server with the current IP address after it restarts.";
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

//cfg.htm
textArray21[iIndex++]="Export current configuration as file (CFG.bin)";
textArray21[iIndex++]='<input type=button value="Export" onClick="return ExportCFG()">';
textArray21[iIndex++]="Import previous configuration from file (CFG.bin)";
textArray21[iIndex++]="Select a file:";
// Begin don't translate
textArray21[iIndex++]='<input type=button name=temp1 value="Import" onClick="return ImportCFG()">';
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
	//crestart.htm
textArray51[iIndex++] = "Upgrade completed successfully!";
textArray51[iIndex++] = "After importing the CFG, the print server will automatically restart, please wait a few moments.";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "Loading Factory Defaults ...";
textArray6[iIndex++] = "After loading the default settings, the print server will automatically restart.<BR><BR>You will be redirected to the Status page when the print server has been restarted.";
iIndex = 0;

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=iso-8859-1">');
}

// out of English