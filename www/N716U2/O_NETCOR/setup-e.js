
//vaiable
menuindex = 0;
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : English
menuArray=['NP302','System Info.','System Conf.','Print Conf.','Network Conf.','Misc','Need Support'];
tabArray=['Printer','Server IP','NetWare','AppleTalk','SNMP','SMB'];

//csystem.htm 0
headArray[iIndex++] = "Printer";
//csystem.htm
headArray[iIndex++] = "This setup page allows you to configure general system settings of the print server.";
//ctcpip.htm 0
headArray[iIndex++] = "Server IP";
//ctcpip.htm
headArray[iIndex++] = "This setup page allows you to configure TCP/IP settings of the print server.";
//cnetware.htm 0
headArray[iIndex++] = "NetWare Settings";
//cnetware.htm
headArray[iIndex++] = "This setup page allows you to configure the NetWare function of the print server.";
//capple.htm 0
headArray[iIndex++] = "ApplaTalk Settings";
//capple.htm 
headArray[iIndex++] ="This setup page allows you to configure AppleTalk settings of the print server.<br>";
//csnmp.htm 0
headArray[iIndex++] = "SNMP Settings";
//csnmp.htm
headArray[iIndex++] ="This setup page allows you to configure SNMP settings of the print server.";
//csmb.htm 0
headArray[iIndex++] = "SMB Settings";
//csmb.htm
headArray[iIndex++] ="This page displays the printer sharing settings for Microsoft Windows networks.";
iIndex = 0;

//csystem.htm
textArray0[iIndex++]="E-mail Alert Settings";
textArray0[iIndex++]="E-mail Alert:";
textArray0[iIndex++]="SMTP Server IP Address:";
textArray0[iIndex++]="Administrator E-mail Address:";
textArray0[iIndex++]="System Settings";
textArray0[iIndex++]="Print Server Name:";
textArray0[iIndex++]="System Contact:";
textArray0[iIndex++]="System Location:";
//textArray0[iIndex++]="Administrator's Password";
//textArray0[iIndex++]="Password:";
//textArray0[iIndex++]="Re-type Password:";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="Save & Restart" class=input_more_back onClick="return SaveSetting(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
//CPRINTER.htm
textArray1[iIndex++]="Parallel Port - Bi-directional Settings";
textArray1[iIndex++]="Printer Port 1:";
textArray1[iIndex++]='<input type="button" value="Save & Restart" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
//CTCPIP.htm
textArray2[iIndex++]="TCP/IP Settings";
textArray2[iIndex++]="Obtain TCP/IP settings automatically (use DHCP/BOOTP)";
textArray2[iIndex++]="Use the following TCP/IP settings";
textArray2[iIndex++]="IP Address:";
textArray2[iIndex++]="Subnet Mask:";
textArray2[iIndex++]="Default Gateway:";
//crandvoo.htm
textArray2[iIndex++]="Rendezvous Settings";
textArray2[iIndex++]="Rendezvous Settings:";
//textArray2[iIndex++]="Disable";
//textArray2[iIndex++]="Enable";
textArray2[iIndex++]="Service Name:";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="Save & Restart" class=input_more_back onClick="return SaveSetting(';
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
textArray3[iIndex++]='<input type=button value="Save & Restart" class=input_more_back onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
//CSNMP.htm
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
textArray4[iIndex++]='<input type="button" value="Save & Restart" class=input_more_back onClick="return SaveSetting(';
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

// CNETWARE.htm
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
textArray8[iIndex++]='<input type="button" value="Save & Restart" class=input_more_back onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.htm
textArray9[iIndex++]="Workgroup";
textArray9[iIndex++]="Name:";
textArray9[iIndex++]="Shared Name";
textArray9[iIndex++]="Printer:";
textArray9[iIndex++]='<input type="button" value="Save & Restart" class=input_more_back onClick="return CheckSMB(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;

// functions
// CSYSTEM.HTM

// CPRINTER.HTM

// CTCPTP.HTM

// CNETWARE.HTM

// CAPPLE.HTM

// CSNMP.HTM

// CSMB.HTM
function CheckSMB(szURL)
{
	if(document.forms[0].SMBWorkGroup.value == '')
	{
		alert("ERROR! The workgroup name cannot be empty!");
		return false;
	}
	else
	{
		if(document.forms[0].SMBPrint1.value == '')
		{		
			alert("ERROR! The SMB shared printer name cannot be empty!");
		 	return false; 
		}
		else
		{
			document.forms[0].action=szURL;
			document.forms[0].submit(); 
			return false; 
		}
	}
}

// out of English
