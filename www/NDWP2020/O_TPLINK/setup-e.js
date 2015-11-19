
//vaiable
tabindex = 0;
textindex = 0;
textindex10 = 0;		// CWLAN.htm Basic Settings
textindex11 = 0;		// CWLAN.htm Advanced Settings
textindex12 = 0;		// CWLAN.htm Site Survey
var iIndex = 0;

//Language : English
tabArray=['<IMG SRC=images/TL-WPS510U-EN.jpg>','Status','Setup','Misc','Restart','System','Wireless','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>This setup page allows you to configure general system settings of the print server.<br>";
//cwlan.htm
headArray[iIndex++] = "<BR>This setup page allows you to configure wireless settings of the print server.";
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

//csystem.htm
textArray0[iIndex++]="E-mail Alert Settings";
textArray0[iIndex++]="E-mail Alert:";
textArray0[iIndex++]="Disabled";
textArray0[iIndex++]="Enabled";
textArray0[iIndex++]="SMTP Server IP Address:";
textArray0[iIndex++]="Administrator's E-mail Address:";
textArray0[iIndex++]="System Settings";
textArray0[iIndex++]="Print Server Name :";
textArray0[iIndex++]="System Contact :";
textArray0[iIndex++]="System Location :";
textArray0[iIndex++]="Administrator's Password";
textArray0[iIndex++]="Account :";
textArray0[iIndex++]="admin";
textArray0[iIndex++]="Password :";
textArray0[iIndex++]="Confirm password :";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="Save & Restart" onClick="return CheckPwd(';
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
textArray2[iIndex++]="Default Router:";
//crandvoo.htm
textArray2[iIndex++]="Rendezvous Settings";
textArray2[iIndex++]="Rendezvous Settings:";
//textArray2[iIndex++]="Disable";
//textArray2[iIndex++]="Enable";
textArray2[iIndex++]="Service Name:";
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
//CSNMP.htm
textArray4[iIndex++]="SNMP Community Settings";
textArray4[iIndex++]="Support HP WebJetAdmin :";
textArray4[iIndex++]="Disabled";
textArray4[iIndex++]="Enabled";
textArray4[iIndex++]="SNMP Community Name 1 :";
textArray4[iIndex++]="Privilege :";
textArray4[iIndex++]="Read-Only";
textArray4[iIndex++]="Read-Write";
textArray4[iIndex++]="SNMP Community Name 2 :";
textArray4[iIndex++]="Privilege :";
textArray4[iIndex++]="Read-Only";
textArray4[iIndex++]="Read-Write";
textArray4[iIndex++]="SNMP Trap Settings";
textArray4[iIndex++]="Send SNMP Traps :";
textArray4[iIndex++]="Disabled";
textArray4[iIndex++]="Enabled";
textArray4[iIndex++]="Use Authentication Traps :";
textArray4[iIndex++]="Disabled";
textArray4[iIndex++]="Enabled";
textArray4[iIndex++]="Trap Address 1 :";
textArray4[iIndex++]="Trap Address 2 :";
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
textArray7[iIndex++]="Invalid IP Address";
textArray7[iIndex++]="Invalid Subnet Mask Address";
textArray7[iIndex++]="Invalid Gateway Address";
textArray7[iIndex++]="Invalid Polling Time Value";
textArray7[iIndex++]="Invalid Print Server Name";
textArray7[iIndex++]="Invalid File Server Name";
textArray7[iIndex++]="DHCP/BOOTP Server not found";
textArray7[iIndex++]="Invalid SNMP Trap IP Address";
textArray7[iIndex++]="Setup password and confirmed do not match";
textArray7[iIndex++]="Wrong firmware or upgrade failed";
textArray7[iIndex++]="Failed in importing the CFG file";
textArray7[iIndex++]="";
textArray7[iIndex++]="Go Back";
iIndex = 0;

// CNETWARE.htm
textArray8[iIndex++]="General Settings";
textArray8[iIndex++]="Printer Server Name :";
textArray8[iIndex++]="Polling Time :";
textArray8[iIndex++]="&nbsp;seconds (min: 3 seconds, max: 29 seconds)";
textArray8[iIndex++]="Logon Password :";
textArray8[iIndex++]="NetWare NDS Settings";
textArray8[iIndex++]="Use NDS Mode :";
textArray8[iIndex++]="Disabled";
textArray8[iIndex++]="Enabled";
textArray8[iIndex++]="Name of the NDS Tree :";
textArray8[iIndex++]="Name of the NDS Context :";
textArray8[iIndex++]="NetWare Bindery Settings";
textArray8[iIndex++]="Use Bindery Mode :";
textArray8[iIndex++]="Disabled";
textArray8[iIndex++]="Enabled";
textArray8[iIndex++]="Name of the File Server :";
textArray8[iIndex++]="File Server not found !";
textArray8[iIndex++]='<input type="button" value="Save & Restart" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.htm
textArray9[iIndex++]="Workgroup";
textArray9[iIndex++]="Name:";
textArray9[iIndex++]="Shared Name";
textArray9[iIndex++]="Printer:";
textArray9[iIndex++]='<input type="button" value="Save & Restart" onClick="return SaveSetting(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;

// CWLAN.htm Basic Settings
textArray10[iIndex++]="Basic Settings";
textArray10[iIndex++]="Network Type :";
textArray10[iIndex++]="SSID :";
textArray10[iIndex++]="Channel :";
//textArray10[iIndex++]="Wireless Country Code :";
//textArray10[iIndex++]="1 - 11, United States, Canada, Ukraine, China";
//textArray10[iIndex++]="1 - 13, Europe (ETSI), except France and Spain";
//textArray10[iIndex++]="10 - 13, France, Singapore";
//textArray10[iIndex++]="10 - 11, Spain, Mexico";
//textArray10[iIndex++]="Modifying Wireless Country Code must obey local (zoning) telecommunication laws.";
//textArray10[iIndex++]="Please save and restart it to apply to this modification.";
//textArray10[iIndex++]='<input type="button" value="Save & Restart" onClick="return SaveSettingCWLAN(';
// Begin don't translate
//textArray10[iIndex++]="'RESTART.HTM');";
//textArray10[iIndex++]='">';
textArray10[iIndex++]="Transmission Rate :";
textArray10[iIndex++]="Automatic";
textArray10[iIndex++]="Wireless Mode :";
textArray10[iIndex++]="B/G Mixed";
textArray10[iIndex++]="B Only";
textArray10[iIndex++]="G Only";
textArray10[iIndex++]="B/G/N Mixed";
textArray10[iIndex++]="N Only";
iIndex = 0;
// End don't translate

//CWLAN.htm Advanced Settings
textArray11[iIndex++]="Advanced Settings";
textArray11[iIndex++]="Security Type";
textArray11[iIndex++]="Disable";
textArray11[iIndex++]="WEP";
textArray11[iIndex++]="Key Index :";
textArray11[iIndex++]="Encryption Type :";
textArray11[iIndex++]="64-bit (10 hex digits)";
textArray11[iIndex++]="64-bit (5 characters)";
textArray11[iIndex++]="128-bit (26 hex digits)";
textArray11[iIndex++]="128-bit (13 characters)";
textArray11[iIndex++]="Key 1 :";
textArray11[iIndex++]="Key 2 :";
textArray11[iIndex++]="Key 3 :";
textArray11[iIndex++]="Key 4 :";
textArray11[iIndex++]="Authentication :";
textArray11[iIndex++]="Open System";
textArray11[iIndex++]="Shared Key";
//textArray11[iIndex++]="Auto";
textArray11[iIndex++]="WPA-PSK";
textArray11[iIndex++]="WPA2-PSK";
textArray11[iIndex++]="Encryption Type :";
textArray11[iIndex++]="Network Security key :";
textArray11[iIndex++]="( 8 to 63 characters ( ASCII ), or 64 hexadecimal digits ( 0 to 9, A to F ) )";
// Translate                                  only "Save & Restart" is to be translated
textArray11[iIndex++]='<input type="button" value="Save & Restart" onClick="return SaveSettingCWLAN(';
// Begin don't translate
textArray11[iIndex++]="'RESTART.HTM');";
textArray11[iIndex++]='">';
iIndex = 0;
// End don't translate

//CWLAN.htm Site Survey
textArray12[iIndex++]="Site Survey";
textArray12[iIndex++]="SSID";
textArray12[iIndex++]="MAC address";
textArray12[iIndex++]="Channel";
textArray12[iIndex++]="Mode";
textArray12[iIndex++]="Security";
textArray12[iIndex++]="Signal Strength(dBm)";
textArray12[iIndex++]='<input name="refresh" onclick="return OnSiteSurvey()" type=button value=" Refresh " style="font-family: Verdana; font-size: 11px;">';
iIndex = 0;

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=iso-8859-1">');
}

// functions
// CSYSTEM.HTM
function CheckPwd(szURL)
{
 	if(document.forms[0].SetupPWD.value != document.forms[0].ConfirmPWD.value && document.forms[0].SetupPWD.value != "ZO__I-SetupPassword" )
	{
		alert("Administrator's Password and Re-type Password do not match !");
		return false;
	}
	document.forms[0].action=szURL;
	document.forms[0].submit();
	return false;
}

// CPRINTER.HTM
function CheckLPRQueueName(szURL)
{
	if ((document.forms[0].LPRQueue1.value == '')
		||(document.forms[0].LPRQueue2.value == '')
		||(document.forms[0].LPRQueue3.value == ''))
	{
		alert("ERROR! The LPR queue name cannot be empty!");
	 	return false; 
	}
	else
	{
		if ((document.forms[0].LPRQueue1.value == document.forms[0].LPRQueue2.value)
			||(document.forms[0].LPRQueue2.value == document.forms[0].LPRQueue3.value)
			||(document.forms[0].LPRQueue1.value == document.forms[0].LPRQueue3.value))
	 	{
	 		alert("ERROR! The LPR queue names cannot be duplicate!");
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

// CWLAN.HTM
function OnSiteSurvey()
{
	newwindow=window.open("WLANTMP0.HTM","","toolbar=0,location=0,directories=0,status=0,menubar=0,width=700,height=400,scrollbars=1");
	return false;
}

// out of English
