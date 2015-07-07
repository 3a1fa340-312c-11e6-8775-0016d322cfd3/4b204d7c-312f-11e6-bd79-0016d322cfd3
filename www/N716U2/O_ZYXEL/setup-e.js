
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : English
tabArray=['Status','Setup','Misc','Restart','System','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>This setup page allows you to configure general system settings of the print server.<br>";
//ctcpip.htm
headArray[iIndex++] = "<BR>This setup page allows you to configure TCP/IP settings of the print server.";
//cnetware.htm
headArray[iIndex++] = "<BR>This setup page allows you to configure the NetWare function of the print server.";
//capple.htm 
headArray[iIndex++] ="<BR>This setup page allows you to configure AppleTalk settings of the print server.<br>";
//csnmp.htm
headArray[iIndex++] ="<BR>This setup page allows you to configure SNMP settings of the print server.";
//csmp.htm
headArray[iIndex++] ="<BR>This page displays the printer sharing settings for Microsoft Windows networks.";
iIndex = 0;

// CSYSTEM.HTM
textArray0[iIndex++]="System Settings";
textArray0[iIndex++]="Print Server Name:";
textArray0[iIndex++]="System Contact:";
textArray0[iIndex++]="System Location:";
textArray0[iIndex++]="Administrator's Password";
textArray0[iIndex++]="Password:";
textArray0[iIndex++]="Re-type Password:";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="Save & Restart" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
// CPRINTER.HTM
textArray1[iIndex++]="Parallel Port - Bi-directional Settings";
textArray1[iIndex++]="Printer Port 1:";
textArray1[iIndex++]='<input type="button" value="Save & Restart" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
// CTCPIP.HTM
textArray2[iIndex++]="TCP/IP Settings";
textArray2[iIndex++]="Obtain TCP/IP settings automatically (use DHCP/BOOTP)";
textArray2[iIndex++]="Use the following TCP/IP settings";
textArray2[iIndex++]="IP Address:";
textArray2[iIndex++]="Subnet Mask:";
textArray2[iIndex++]="Default Router:";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="Save & Restart" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="AppleTalk Settings";
textArray3[iIndex++]="AppleTalk Zone:";
textArray3[iIndex++]="Port Name:";
textArray3[iIndex++]="Printer Configuration";
textArray3[iIndex++]="Type:";
textArray3[iIndex++]="Binary Protocol:";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="Save & Restart" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
// CSNMP.HTM
textArray4[iIndex++]="SNMP Community Settings";
textArray4[iIndex++]="Support HP WebJetAdmin :";
textArray4[iIndex++]="SNMP Community Name 1:";
textArray4[iIndex++]="Privilege:  ";
textArray4[iIndex++]="SNMP Community Name 2:";
textArray4[iIndex++]="Privilege:";
textArray4[iIndex++]="SNMP Trap Settings";
textArray4[iIndex++]="Send SNMP Traps:";
textArray4[iIndex++]="Use Authentication Traps:";
textArray4[iIndex++]="Trap Address 1:";
textArray4[iIndex++]="Trap Address 2:";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="Save & Restart" onClick="return SaveSetting(';
// Begin don't translate
textArray4[iIndex++]="'RESTART.HTM');";
textArray4[iIndex++]='">';
iIndex = 0;
// End don't translate
//keyhelp.htm
//textArray5[iIndex++]="<b>WEP Key Format</b>";
//textArray5[iIndex++]="An alphanumeric character is 'a' through 'z', 'A' through 'Z', and '0' through '9'.";
//textArray5[iIndex++]="A hexadecimal digit is '0' through '9' and 'A' through 'F'.";
//textArray5[iIndex++]="Depending on the key format you select:";
//textArray5[iIndex++]="For 64-bit (sometimes called 40-bit) WEP encryption, enter the key which contains 5 alphanumeric characters or 10 hexadecimal digits. For example: AbZ12 (alphanumeric format) or ABCDEF1234 (hexadecimal format).";
//textArray5[iIndex++]="For 128-bit WEP encryption, enter the key which contains 13 alphanumeric characters or 26 hexadecimal digits.";
// Translate                                  only "Close" is to be translated
textArray5[iIndex++]='<INPUT TYPE=button VALUE=" Close " onClick="window.close()">';
iIndex = 0;
//browser.htm
//textArray6[iIndex++]="SSID";
//textArray6[iIndex++]="AP's MAC Address or BSSID";
//textArray6[iIndex++]="Channel";
//textArray6[iIndex++]="Type";
//textArray6[iIndex++]="WEP/WPA-PSK";
//textArray6[iIndex++]="Signal Strength";
// Translate                                  only "Rescan" is to be translated
//textArray6[iIndex++]='<INPUT TYPE=submit VALUE="Rescan">';
// Translate                                  only "Close" is to be translated
//textArray6[iIndex++]='<INPUT TYPE=button VALUE=" Close " onClick="window.close()">';
//iIndex = 0;
// End don't translate
// ERROR.htm
textArray7[iIndex++]="ERROR";
textArray7[iIndex++]="Go Back";
iIndex = 0;

// CNETWARE.HTM
textArray8[iIndex++]="General Settings";
textArray8[iIndex++]="Printer Server Name:";
textArray8[iIndex++]="Polling Time:";
textArray8[iIndex++]="&nbsp;seconds (min: 3, max: 29 seconds)";
textArray8[iIndex++]="Logon Password:";
textArray8[iIndex++]="NetWare NDS Settings";
textArray8[iIndex++]="Use NDS Mode:";
textArray8[iIndex++]="Name of the NDS Tree:";
textArray8[iIndex++]="Name of the NDS Context:";
textArray8[iIndex++]="NetWare Bindery Settings";
textArray8[iIndex++]="Use Bindery Mode:";
textArray8[iIndex++]="Name of the File Server:";
textArray8[iIndex++]='<input type="button" value="Save & Restart" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.HTM
textArray9[iIndex++]="Workgroup";
textArray9[iIndex++]="Name:";
textArray9[iIndex++]="Shared Name";
textArray9[iIndex++]="Printer:";
textArray9[iIndex++]='<input type="button" value="Save & Restart" onClick="return SaveSetting(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;
// out of English
