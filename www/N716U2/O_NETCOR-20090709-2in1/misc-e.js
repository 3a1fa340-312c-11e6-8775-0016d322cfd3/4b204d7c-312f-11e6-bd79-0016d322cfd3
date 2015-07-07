

//vaiable
menuindex = 0;
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : English
menuArray=['NP302','System Info.','System Conf.','Print Conf.','Network Conf.','Misc','Need Support'];
tabArray=['Restart','Firmware Upgrade','Factory Default','Password'];

//reset.htm 0
headArray[iIndex++] ="Restart";
//reset.htm
headArray[iIndex++] ="This page allows you to restart the print server.";
//upgrade.htm 0
headArray[iIndex++] ="Firmware Upgrade";
//upgrade.htm
headArray[iIndex++] ="This page allows you to upgrade the firmware of the print server.<br>Note: please make sure the firmware is correct before you proceed. If you do not know which firmware file you should use, please contact your local dealer for technical support.";
//default.htm 0
headArray[iIndex++] ="Factory Default";
//default.htm
headArray[iIndex++] ="Reload all default settings in the print server. All current settings will be erased.";
//password.htm 0
headArray[iIndex++] ="Password";
//password.htm
headArray[iIndex++] ="This page allows you to configure the administrator's password of the print server.";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "Reload all default settings in the print server. Warning! All current settings will be erased.";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;OK&nbsp;&nbsp;" class=input_more_back onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="Firmware Upgrade";
textArray2[iIndex++]="Select Firmware Directory and File:";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="Save & Restart" class=input_more_back onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>Restart The Print Server</B></FONT>";
textArray3[iIndex++]="Do you want to save settings and restart the print server now?";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;OK&nbsp;&nbsp;" class=input_more_back onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

//password.htm
textArray3a[iIndex++]="Administrator's Password";
textArray3a[iIndex++]="Password:";
textArray3a[iIndex++]="Re-type Password:";
// Translate                               Only OK is to be translated
textArray3a[iIndex++]='<input type=button value="Save & Restart" class=input_more_back onClick="return CheckPwd(';
// Begin don't translate
textArray3a[iIndex++]="'RESTART.HTM');";
textArray3a[iIndex++]='">';
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

	//support.htm
textArray7[iIndex++] = "Support";
textArray7[iIndex++] = "NetCore China....";
textArray7[iIndex++] = "Paragraph 2";
textArray7[iIndex++] = "For more information, please call 400-810-1616.";
textArray7[iIndex++] = "NetCore Official";
textArray7[iIndex++] = "Whole China";
textArray7[iIndex++] = "NetCore Official Web Site (www.netcoretec.com)";
textArray7[iIndex++] = "Paragraph";
textArray7[iIndex++] = "NetCore North";
textArray7[iIndex++] = "North China";
textArray7[iIndex++] = "NetCore North Web Site (www.netcore.com.cn)";
textArray7[iIndex++] = "Paragraph";
iIndex = 0;
// out of English
