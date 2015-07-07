//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

tabArray=['<IMG SRC=images/MPS110U-SC.gif>','Status','Setup','Misc','Restart','System','Printer','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
//Language : English

//system.htm
headArray[iIndex++] = "<BR>This page displays the general system information of the print server.<BR>";
//printer.htm
headArray[iIndex++] = "<BR>This page displays the information of the printer which is currently connected to the print server.<BR>Note: If your printer does not support bi-directional function, some information may not be correctly displayed correctly.";
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


// out of English
