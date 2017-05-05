//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

//tabArray=['PU211 USB Port Print Server','Status','Setup','Misc','Restart','System','Printer','TCPIP','Services','NetWare','AppleTalk','SNMP','SMB',''];
tabArray=['System','Printer','TCPIP','Services','NetWare','AppleTalk','SNMP','SMB',''];
//Language : English

//system.htm
headArray[iIndex++] = "<BR>This page displays the general system information of the print server.<BR>";
//printer.htm
headArray[iIndex++] = "<BR>This page displays the information of the printer which is currently connected to the print server.<BR>Note: If your printer does not support bi-directional function, some information may not be correctly displayed.";
//tcpip.htm
headArray[iIndex++] = "<BR>This page displays the current TCP/IP settings of the print server.<BR>";
//services.htm
headArray[iIndex++] = "<BR>This page displays the service information of the print server.<BR>";
//netware.htm
headArray[iIndex++] = "<BR>This page displays the current NetWare settings of the print server. <BR>";
//apple.htm
headArray[iIndex++] = "<BR>This page displays the current AppleTalk settings of the print server.<BR>";
//snmp.htm
headArray[iIndex++] = "<BR>This page displays the current SNMP settings of the print server.<BR>";
//Smb.htm
headArray[iIndex++] = "<BR>This page displays the printer sharing settings for Microsoft Windows networks.<BR>";
iIndex = 0;



//system.htm
textArray0[iIndex++]="System Information";
textArray0[iIndex++]="Print Server Name :";
textArray0[iIndex++]="System Contact :";
textArray0[iIndex++]="System Location :";
textArray0[iIndex++]="System Up Time :";
textArray0[iIndex++]="Firmware Version :";
textArray0[iIndex++]="MAC Address :";
textArray0[iIndex++]="E-mail Alert :";
textArray0[iIndex++]="Disabled";
textArray0[iIndex++]="Enabled";
//PRINTJOB.htm
textArray0[iIndex++]="Print Jobs";
textArray0[iIndex++]="Job";
textArray0[iIndex++]="User";
textArray0[iIndex++]="Elapsed Time";
textArray0[iIndex++]="Protocol";
textArray0[iIndex++]="Port";
textArray0[iIndex++]="Status";
textArray0[iIndex++]="Bytes Printed";
textArray0[iIndex++]="View Job Log";
iIndex = 0;

//Printer.htm
textArray1[iIndex++]="Printer Information";
textArray1[iIndex++]="Manufacturer :";
textArray1[iIndex++]="Model Number :";
textArray1[iIndex++]="Printing Language Supported :";
textArray1[iIndex++]="Current Status :";
textArray1[iIndex++]="Waiting for job";
textArray1[iIndex++]="Paper Out";
textArray1[iIndex++]="Offline";
textArray1[iIndex++]="Printing";
textArray1[iIndex++]="Print Speed :";
textArray1[iIndex++]="Fast";
textArray1[iIndex++]="Middle";
textArray1[iIndex++]="Slow";
//textArray1[iIndex++]="LPR Queue Name :";
iIndex = 0;

//NETWARE.htm
textArray2[iIndex++]="General Settings";
textArray2[iIndex++]="Print Server Name :";
textArray2[iIndex++]="Polling Time :";
textArray2[iIndex++]="seconds";
textArray2[iIndex++]="NetWare NDS Settings";
textArray2[iIndex++]="Use NDS Mode :";
textArray2[iIndex++]="Disabled";
textArray2[iIndex++]="Enabled";
textArray2[iIndex++]="Name of the NDS Tree :";
textArray2[iIndex++]="Name of the NDS Context :";
textArray2[iIndex++]="Current Status:";
textArray2[iIndex++]="Disconnected";
textArray2[iIndex++]="Connected";
textArray2[iIndex++]="NetWare Bindery Settings";
textArray2[iIndex++]="Use Bindery Mode :";
textArray2[iIndex++]="Disabled";
textArray2[iIndex++]="Enabled";
textArray2[iIndex++]="Name of the File Server :";
textArray2[iIndex++]="Current Status :";
textArray2[iIndex++]="Disconnected";
textArray2[iIndex++]="Connected";
iIndex = 0;
//tcpip.htm
textArray3[iIndex++]="TCP/IP Settings";
textArray3[iIndex++]="Use DHCP/BOOTP :";
textArray3[iIndex++]="IP Address :";
textArray3[iIndex++]="Subnet Mask :";
textArray3[iIndex++]="Gateway :";
//randvoo.htm
textArray3[iIndex++]="Rendezvous Settings";
textArray3[iIndex++]="Rendezvous Settings :";
//textArray3[iIndex++]="Disabled";
//textArray3[iIndex++]="Enabled";
textArray3[iIndex++]="Service Name :";
iIndex = 0;
//APPLE.htm
textArray4[iIndex++]="AppleTalk Settings";
textArray4[iIndex++]="AppleTalk Zone Name :";
textArray4[iIndex++]="Printer Information";
textArray4[iIndex++]="Port Name :";
textArray4[iIndex++]="Printer Type :";
textArray4[iIndex++]="Data Format :";
iIndex = 0;
//SNMP.htm
textArray5[iIndex++]="SNMP Community Settings";
textArray5[iIndex++]="SNMP Community 1 :";
textArray5[iIndex++]="Read-Only";
textArray5[iIndex++]="Read-Write";
textArray5[iIndex++]="SNMP Community 2 :";
textArray5[iIndex++]="Read-Only";
textArray5[iIndex++]="Read-Write";
textArray5[iIndex++]="SNMP Trap Settings";
textArray5[iIndex++]="Send SNMP Traps :";
textArray5[iIndex++]="Disabled";
textArray5[iIndex++]="Enabled";
textArray5[iIndex++]="Use Authentication Traps :";
textArray5[iIndex++]="Disabled";
textArray5[iIndex++]="Enabled";
textArray5[iIndex++]="Trap Address 1 :";
textArray5[iIndex++]="Trap Address 2 :";
iIndex = 0;

//JOBLOG.htm
// Translate                                  only "Refresh " is to be translated
textArray6[iIndex++]='<input type=button value=" Refresh " onClick="window.location.reload()">';
textArray6[iIndex++]="Print Jobs";
textArray6[iIndex++]="Job";
textArray6[iIndex++]="User";
textArray6[iIndex++]="Elapsed Time";
textArray6[iIndex++]="Protocol";
textArray6[iIndex++]="Port";
textArray6[iIndex++]="Status";
textArray6[iIndex++]="Bytes Printed";
// Translate                                  only "Close " is to be translated
textArray6[iIndex++]='<input type=button value=" Close " onClick="window.close()">';
iIndex = 0;

//SMB.htm
textArray7[iIndex++]="Workgroup";
textArray7[iIndex++]="Name :";
textArray7[iIndex++]="Shared Printer Name";
textArray7[iIndex++]="Printer :";
iIndex = 0;

//SERVICES.htm
textArray8[iIndex++]="Printing Method";
textArray8[iIndex++]="Use NetWare Bindery :";
textArray8[iIndex++]="Disabled";
textArray8[iIndex++]="Enabled";
textArray8[iIndex++]="Use NetWare NDS :";
textArray8[iIndex++]="Disabled";
textArray8[iIndex++]="Enabled";
textArray8[iIndex++]="Use LPR/LPD :";
textArray8[iIndex++]="Disabled";
textArray8[iIndex++]="Enabled";
textArray8[iIndex++]="Use AppleTalk :";
textArray8[iIndex++]="Disabled";
textArray8[iIndex++]="Enabled";
textArray8[iIndex++]="Use IPP :";
textArray8[iIndex++]="Disabled";
textArray8[iIndex++]="Enabled";
textArray8[iIndex++]="Use SMB :";
textArray8[iIndex++]="Disabled";
textArray8[iIndex++]="Enabled";
textArray8[iIndex++]="Services";
textArray8[iIndex++]="Telnet :";
textArray8[iIndex++]="Disabled";
textArray8[iIndex++]="Enabled";
textArray8[iIndex++]="SNMP :";
textArray8[iIndex++]="Disabled";
textArray8[iIndex++]="Enabled";
textArray8[iIndex++]="E-mail and EOF Alert :";
textArray8[iIndex++]="Disabled";
textArray8[iIndex++]="Enabled";
textArray8[iIndex++]="HTTP :";
textArray8[iIndex++]="Disabled";
textArray8[iIndex++]="Enabled";

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
function BodyPreloadImages()
{
	document.write("<body onload=MM_preloadImages('imgenus/MenuBtn-E-setup2.gif','imgenus/MenuBtn-E-other2.gif','imgenus/MenuBtn-E-restart2.gif')>");
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
	document.write('<td><img src="imgenus/MenuBtn-E-status3.gif" width="92" height="36" /></td>');
	document.write("<td><a href=CSYSTEM.HTM target=_parent onmouseover=MM_swapImage('Image14','','imgenus/MenuBtn-E-setup2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgenus/MenuBtn-E-setup1.gif name=Image14 width=93 height=36 border=0 id=Image14></a></td>");
	document.write("<td><a href=DEFAULT.HTM target=_parent onmouseover=MM_swapImage('Image15','','imgenus/MenuBtn-E-other2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgenus/MenuBtn-E-other1.gif name=Image15 width=93 height=36 border=0 id=Image15></a></td>");
	document.write("<td><a href=RESET.HTM target=_parent onmouseover=MM_swapImage('Image16','','imgenus/MenuBtn-E-restart2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgenus/MenuBtn-E-restart1.gif name=Image16 width=93 height=36 border=0 id=Image16></a></td>");
}

// out of English
