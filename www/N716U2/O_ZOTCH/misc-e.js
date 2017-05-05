

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : English
//tabArray=['PU211 USB Port Print Server','Status','Setup','Misc','Restart','Factory Default','Firmware Upgrade'];
tabArray=['Factory Default','Firmware Upgrade'];

	//upgrade.htm
headArray[iIndex++] ="<br>This page allows you to upgrade the firmware of the print server.<br>Note: please make sure the firmware is correct before you proceed. If you do not know which firmware file you should use, please contact your local dealer for technical support.";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "Click <b>Factory Default</b> then <b>OK</b> to reload all default settings in the print server.<BR><FONT CLASS=F1 COLOR=#FF3300><B>Warning! All current settings will be erased.</B></FONT>";
textArray1[iIndex++] = "Load default, excluding TCP/IP settings. (Recommended)";
textArray1[iIndex++] = "You can still connect the print server with the current IP address after it restarts.";
textArray1[iIndex++] = "Load default, including TCP/IP settings.";
textArray1[iIndex++] = "You cannot connect the print server with the current IP address after it restarts.";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;OK&nbsp;&nbsp;" onClick="return SaveIPSetting(';
// Begin don't translate
textArray1[iIndex++] = "'DRESTART.HTM');";
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

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=iso-8859-1">');
}

// Title or Model Name
function TitleModelName()
{
	document.write('<title>PU211 USB Port Print Server</title>');
}

// MM_preloadImages
function BodyPreloadImages2()
{
	document.write("<body onload=OnLoadTasks()>");
}

function OnLoadTasks()
{
	MM_preloadImages('imgenus/MenuBtn-E-setup2.gif','imgenus/MenuBtn-E-other2.gif','imgenus/MenuBtn-E-restart2.gif');
	GoToLastPage();
}

// mainView-Title
function MainViewTitle()
{
	document.write('<tr><td><img src="images/mainView-2.gif" width="301" height="32" /></td>');
	document.write('<td><img src="imgenus/mainView-Title-E.gif" width="431" height="32">');
	document.write('</td></tr>');
}

// Row MenuBtn
function RowMenuBtn()
{
	document.write("<td><a href=SYSTEM.HTM target=_parent onmouseover=MM_swapImage('Image14','','imgenus/MenuBtn-E-status2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgenus/MenuBtn-E-status1.gif name=Image14 width=93 height=36 border=0 id=Image14></a></td>");
	document.write("<td><a href=CSYSTEM.HTM target=_parent onmouseover=MM_swapImage('Image15','','imgenus/MenuBtn-E-setup2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgenus/MenuBtn-E-setup1.gif name=Image15 width=93 height=36 border=0 id=Image15></a></td>");
	document.write('<td><img src="imgenus/MenuBtn-E-other3.gif" width="92" height="36" /></td>');
	document.write("<td><a href=RESET.HTM target=_parent onmouseover=MM_swapImage('Image16','','imgenus/MenuBtn-E-restart2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgenus/MenuBtn-E-restart1.gif name=Image16 width=93 height=36 border=0 id=Image16></a></td>");
}

function RowMenuBtn4()
{
	document.write("<td><a href=SYSTEM.HTM target=_parent onmouseover=MM_swapImage('Image14','','imgenus/MenuBtn-E-status2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgenus/MenuBtn-E-status1.gif name=Image14 width=93 height=36 border=0 id=Image14></a></td>");
	document.write("<td><a href=CSYSTEM.HTM target=_parent onmouseover=MM_swapImage('Image15','','imgenus/MenuBtn-E-setup2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgenus/MenuBtn-E-setup1.gif name=Image15 width=93 height=36 border=0 id=Image15></a></td>");
	document.write("<td><a href=DEFAULT.HTM target=_parent onmouseover=MM_swapImage('Image16','','imgenus/MenuBtn-E-other2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgenus/MenuBtn-E-other1.gif name=Image16 width=93 height=36 border=0 id=Image16></a></td>");
	document.write('<td><img src="imgenus/MenuBtn-E-restart3.gif" width="92" height="36" /></td>');
}

// out of English
