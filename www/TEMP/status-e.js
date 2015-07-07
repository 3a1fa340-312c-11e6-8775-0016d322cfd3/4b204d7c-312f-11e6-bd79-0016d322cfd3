//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

//tabArray=['<IMG SRC=images/LOGO.gif>','Status','Setup','Misc','Restart','System','Printer','Wireless','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
tabArray=['Mini-103MN','Status','Setup','Misc','Restart','System','Printer','Wireless','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
//Language : English

//system.htm
headArray[iIndex++] = "<BR>This page displays the general system information of the print server.<BR>";
//printer.htm
headArray[iIndex++] = "<BR>This page displays the information of the printer which is currently connected to the print server.<BR>Note: If your printer does not support bi-directional function, some information may not be displayed correctly.";
//wlan.htm
headArray[iIndex++] = "<BR>This page displays the current wireless settings and the status of the print server.";
//tcpip.htm
headArray[iIndex++] = "<BR>This page displays the current TCP/IP settings of the print server.<BR>";
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
textArray0[iIndex++]="Print Server Name:";
textArray0[iIndex++]="System Contact:";
textArray0[iIndex++]="System Location:";
textArray0[iIndex++]="System Up Time:";
textArray0[iIndex++]="Firmware Version:";
textArray0[iIndex++]="MAC Address:";
textArray0[iIndex++]="E-mail Alert:";
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
textArray1[iIndex++]="Manufacturer";
textArray1[iIndex++]="Model Number";
textArray1[iIndex++]="Printing Language Supported";
textArray1[iIndex++]="Current Status";
textArray1[iIndex++]="Waiting for job";
textArray1[iIndex++]="Paper Out";
textArray1[iIndex++]="Offline";
textArray1[iIndex++]="Printing";
iIndex = 0;

//NETWARE.htm
textArray2[iIndex++]="General Settings";
textArray2[iIndex++]="Print Server Name:";
textArray2[iIndex++]="Polling Time:";
textArray2[iIndex++]="seconds";
textArray2[iIndex++]="NetWare NDS Settings";
textArray2[iIndex++]="Use NDS Mode:";
textArray2[iIndex++]="Disabled";
textArray2[iIndex++]="Enabled";
textArray2[iIndex++]="Name of the NDS Tree:";
textArray2[iIndex++]="Name of the NDS Context:";
textArray2[iIndex++]="Current Status:";
textArray2[iIndex++]="Disconnected";
textArray2[iIndex++]="Connected";
textArray2[iIndex++]="NetWare Bindery Settings";
textArray2[iIndex++]="Use Bindery Mode:";
textArray2[iIndex++]="Disabled";
textArray2[iIndex++]="Enabled";
textArray2[iIndex++]="Name of the File Server:";
textArray2[iIndex++]="Current Status:";
textArray2[iIndex++]="Disconnected";
textArray2[iIndex++]="Connected";
iIndex = 0;
//tcpip.htm
textArray3[iIndex++]="TCP/IP Settings";
textArray3[iIndex++]="Use DHCP/BOOTP:";
textArray3[iIndex++]="IP Address:";
textArray3[iIndex++]="Subnet Mask:";
textArray3[iIndex++]="Gateway:";
//randvoo.htm
textArray3[iIndex++]="Rendezvous Settings";
textArray3[iIndex++]="Rendezvous Settings:";
//textArray3[iIndex++]="Disable";
//textArray3[iIndex++]="Enable";
textArray3[iIndex++]="Service Name:";
iIndex = 0;
//APPLE.htm
textArray4[iIndex++]="AppleTalk Settings";
textArray4[iIndex++]="AppleTalk:";
textArray4[iIndex++]="Printer Information";
textArray4[iIndex++]="Port Name:";
textArray4[iIndex++]="Printer Type:";
textArray4[iIndex++]="Data Format:";
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
textArray7[iIndex++]="Name:";
textArray7[iIndex++]="Shared Name";
textArray7[iIndex++]="Printer:";
iIndex = 0;

//WLAN.htm
textArray8[iIndex++]="<BR><BR><FONT COLOR=RED>Warning ! The print server is now in Diagnostic Mode.</FONT><BR>";
textArray8[iIndex++]="Wireless Information";
textArray8[iIndex++]="Mode:";
textArray8[iIndex++]="Client mode";
textArray8[iIndex++]="AP mode (Access Point)";
textArray8[iIndex++]="Type:";
textArray8[iIndex++]="Infrastructure";
textArray8[iIndex++]="Ad-Hoc";
textArray8[iIndex++]="AP's MAC Address:";
textArray8[iIndex++]="SSID:";
textArray8[iIndex++]="Channel Number:";
textArray8[iIndex++]="Transmit Mode:";
textArray8[iIndex++]="B only";
textArray8[iIndex++]="G only";
textArray8[iIndex++]="B/G/N Mixed";
textArray8[iIndex++]="B/G Mixed";
textArray8[iIndex++]="Transmission Rate:";
textArray8[iIndex++]="Rx Signal Intensity:";
textArray8[iIndex++]="Link Quality:";
textArray8[iIndex++]="Authentication Type:";
textArray8[iIndex++]="Shared Key";
textArray8[iIndex++]="WPA-PSK";
textArray8[iIndex++]="WPA2-PSK";
textArray8[iIndex++]="Open System";
textArray8[iIndex++]="Encryption:";
textArray8[iIndex++]="WEP 64-bit";
textArray8[iIndex++]="WEP 128-bit";
textArray8[iIndex++]="None";
textArray8[iIndex++]="Disabled";
iIndex = 0;

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=iso-8859-1">');
}

// out of English
