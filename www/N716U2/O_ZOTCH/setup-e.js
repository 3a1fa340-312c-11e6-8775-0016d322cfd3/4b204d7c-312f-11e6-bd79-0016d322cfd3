
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : English
//tabArray=['PU211 USB Port Print Server','Status','Setup','Misc','Restart','System','TCP/IP','Services','NetWare','AppleTalk','SNMP','SMB','',''];
tabArray=['System','TCP/IP','Services','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>This setup page allows you to configure general system settings of the print server.<br>";
//cprinter.htm
headArray[iIndex++] = "<BR>This setup page allows you to enable or disable the bi-directional function of each printer port.";
//ctcpip.htm
headArray[iIndex++] = "<BR>This setup page allows you to configure TCP/IP settings of the print server.";
//cservices.htm
headArray[iIndex++] = "<BR>This setup page allows you to configure the services of the print server.";
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
textArray0[iIndex++]="E-mail Alert :";
textArray0[iIndex++]="Disabled";
textArray0[iIndex++]="Enabled";
textArray0[iIndex++]="SMTP Server IP Address:";
textArray0[iIndex++]="Administrator E-mail Address:";
textArray0[iIndex++]="System Settings";
textArray0[iIndex++]="Print Server Name :";
textArray0[iIndex++]="System Contact :";
textArray0[iIndex++]="System Location :";
textArray0[iIndex++]="Administrator's Password";
textArray0[iIndex++]="Account :";
textArray0[iIndex++]="admin";
textArray0[iIndex++]="Password :";
textArray0[iIndex++]="Confirm Password :";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="Save & Restart" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
//CPRINTER.htm
textArray1[iIndex++]="Parallel Port - Bi-directional Settings";
textArray1[iIndex++]="Printer Port :";
textArray1[iIndex++]="Disabled";
textArray1[iIndex++]="Auto Detect";
textArray1[iIndex++]="Print Speed :";
textArray1[iIndex++]="Fast";
textArray1[iIndex++]="Middle";
textArray1[iIndex++]="Slow";
textArray1[iIndex++]="For laser printers, fast is recommended.";
textArray1[iIndex++]="For dot matrix printers, slow is recommended.";
//textArray1[iIndex++]="LPR Queue Name :";
//textArray1[iIndex++]="(Max 12 alphanumeric characters without any blank characters)";
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
textArray2[iIndex++]="IP Address :";
textArray2[iIndex++]="Subnet Mask :";
textArray2[iIndex++]="Default Gateway :";
//crandvoo.htm
textArray2[iIndex++]="Rendezvous Settings";
textArray2[iIndex++]="Rendezvous Settings :";
//textArray2[iIndex++]="Disable";
//textArray2[iIndex++]="Enable";
textArray2[iIndex++]="Service Name :";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="Save & Restart" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="AppleTalk Settings";
textArray3[iIndex++]="AppleTalk Service :";
textArray3[iIndex++]="Disabled";
textArray3[iIndex++]="Enabled";
textArray3[iIndex++]="AppleTalk Zone Name :";
textArray3[iIndex++]="Port Name :";
textArray3[iIndex++]="Printer Configuration";
textArray3[iIndex++]="Type :";
textArray3[iIndex++]="Data Format :";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="Save & Restart" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
//CSNMP.htm
textArray4[iIndex++]="SNMP Community Settings";
textArray4[iIndex++]="SNMP Service :";
textArray4[iIndex++]="Disabled";
textArray4[iIndex++]="Enabled";
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
textArray8[iIndex++]='<input type="button" value="Save & Restart" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.htm
textArray9[iIndex++]="Workgroup";
textArray9[iIndex++]="SMB Service :";
textArray9[iIndex++]="Disabled";
textArray9[iIndex++]="Enabled";
textArray9[iIndex++]="Name :";
textArray9[iIndex++]="Shared Printer Name";
textArray9[iIndex++]="Printer :";
textArray9[iIndex++]='<input type="button" value="Save & Restart" onClick="return CheckSMB(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;

// CSERVICES.htm
textArray10[iIndex++]="Printing Method";
textArray10[iIndex++]="Use LPR/LPD :";
textArray10[iIndex++]="Disabled";
textArray10[iIndex++]="Enabled";
textArray10[iIndex++]="Use AppleTalk :";
textArray10[iIndex++]="Disabled";
textArray10[iIndex++]="Enabled";
textArray10[iIndex++]="Use IPP :";
textArray10[iIndex++]="Disabled";
textArray10[iIndex++]="Enabled";
textArray10[iIndex++]="Use SMB :";
textArray10[iIndex++]="Disabled";
textArray10[iIndex++]="Enabled";
textArray10[iIndex++]="Services";
textArray10[iIndex++]="Telnet :";
textArray10[iIndex++]="Disabled";
textArray10[iIndex++]="Enabled";
textArray10[iIndex++]="SNMP :";
textArray10[iIndex++]="Disabled";
textArray10[iIndex++]="Enabled";
textArray10[iIndex++]="E-mail and EOF Alert :";
textArray10[iIndex++]="Disabled";
textArray10[iIndex++]="Enabled";
textArray10[iIndex++]="HTTP :";
textArray10[iIndex++]="Disabled";
textArray10[iIndex++]="Enabled";
textArray10[iIndex++]="ATTENTION: You can ONLY enable HTTP with the reset button after you disable it.";
textArray10[iIndex++]="And, stopping HTTP will also stop the IPP printing automatically.";
textArray10[iIndex++]='<input type="button" value="Save & Restart" onClick="return SaveServices(';
// Begin don't translate
textArray10[iIndex++]="'RESTART.HTM');";
textArray10[iIndex++]='">';
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
 	if(document.CSYSTEM.SetupPWD.value != document.CSYSTEM.ConfirmPWD.value && document.CSYSTEM.SetupPWD.value != "ZO__I-SetupPassword" )
	{
		alert("Administrator's Password and Confirmed Password do not match !");
		return false;
	}
	document.CSYSTEM.action=szURL;
	document.CSYSTEM.submit();
	return false;
}

// CPRINTER.HTM
function CheckLPRQueueName(szURL)
{
	var ch;
	var lprname1 = document.forms[0].LPRQueue1.value;

	if (document.forms[0].LPRQueue1.value == '')
	{
		alert("ERROR! The LPR queue name cannot be empty!");
	 	return false; 
	}
	else
	{
		for(var i = 0 ; i < lprname1.length ; i++ )
		{
	    	ch = lprname1.charAt(i).toUpperCase();
	
			if(!((ch >= '0' && ch <= '9') ||
				(ch >= 'A' && ch <= 'Z') || 
				(ch >= 'a' && ch <= 'z') || 
				(ch == '-') || (ch == '_')))
			{
		  		alert ("Invalid character " + ch + " in the LPR queue name " + lprname1);
				document.forms[0].LPRQueue1.focus();
				return false;
			}
		}

		document.forms[0].action=szURL;
		document.forms[0].submit(); 
		return false; 
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
	document.write("<td><a href=SYSTEM.HTM target=_parent onmouseover=MM_swapImage('Image14','','imgenus/MenuBtn-E-status2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgenus/MenuBtn-E-status1.gif name=Image14 width=93 height=36 border=0 id=Image14></a></td>");
	document.write('<td><img src="imgenus/MenuBtn-E-setup3.gif" width="92" height="36" /></td>');
	document.write("<td><a href=DEFAULT.HTM target=_parent onmouseover=MM_swapImage('Image15','','imgenus/MenuBtn-E-other2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgenus/MenuBtn-E-other1.gif name=Image15 width=93 height=36 border=0 id=Image15></a></td>");
	document.write("<td><a href=RESET.HTM target=_parent onmouseover=MM_swapImage('Image16','','imgenus/MenuBtn-E-restart2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgenus/MenuBtn-E-restart1.gif name=Image16 width=93 height=36 border=0 id=Image16></a></td>");
}

// out of English
