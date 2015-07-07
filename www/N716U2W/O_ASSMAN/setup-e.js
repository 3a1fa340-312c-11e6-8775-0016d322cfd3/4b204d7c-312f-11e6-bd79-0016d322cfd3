
//vaiable
tabindex = 0;
textindex = 0;
textindex10 = 0;		// CWLAN.htm Basic Settings
textindex11 = 0;		// CWLAN.htm Advanced Settings
textindex12 = 0;		// CWLAN.htm Site Survey
var iIndex = 0;

//Language : English
tabArray=['Print Server','Status','Setup','Misc','Restart','System','Wireless','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>This setup page allows you to configure general system settings of the print server.<br>";
//cprinter.htm
//headArray[iIndex++] = "<BR>This setup page allows you to enable or disable the bi-directional function of each printer port.";
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

// CSYSTEM.HTM
textArray0[iIndex++]="E-mail Alert Settings";
textArray0[iIndex++]="E-mail Alert :";
textArray0[iIndex++]="Disabled";
textArray0[iIndex++]="Enabled";
textArray0[iIndex++]="SMTP Server IP Address :";
textArray0[iIndex++]="Administrator's E-mail Address :";
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
textArray1[iIndex++]="Printer Port :";
textArray1[iIndex++]="Disable";
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
textArray2[iIndex++]="Gateway :";
//crandvoo.htm
textArray2[iIndex++]="Rendezvous Settings";
textArray2[iIndex++]="Rendezvous Settings:";
textArray2[iIndex++]="Disable";
textArray2[iIndex++]="Enable";
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
textArray3[iIndex++]="AppleTalk Zone :";
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
// CSNMP.HTM
textArray4[iIndex++]="SNMP Community Settings";
textArray4[iIndex++]="Support HP WebJetAdmin :";
textArray4[iIndex++]="Disable";
textArray4[iIndex++]="Enable";
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
textArray4[iIndex++]="Disable";
textArray4[iIndex++]="Enable";
textArray4[iIndex++]="Use Authentication Traps :";
textArray4[iIndex++]="Disable";
textArray4[iIndex++]="Enable";
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
textArray7[iIndex++]="";
textArray7[iIndex++]="Go Back";
iIndex = 0;

// CNETWARE.HTM
textArray8[iIndex++]="General Settings";
textArray8[iIndex++]="Printer Server Name :";
textArray8[iIndex++]="Polling Time :";
textArray8[iIndex++]="&nbsp;seconds (min: 3 seconds, max: 29 seconds)";
textArray8[iIndex++]="Logon Password :";
textArray8[iIndex++]="NetWare NDS Settings";
textArray8[iIndex++]="Use NDS Mode :";
textArray8[iIndex++]="Disable";
textArray8[iIndex++]="Enable";
textArray8[iIndex++]="Name of the NDS Tree :";
textArray8[iIndex++]="Name of the NDS Context :";
textArray8[iIndex++]="NetWare Bindery Settings";
textArray8[iIndex++]="Use Bindery Mode :";
textArray8[iIndex++]="Disable";
textArray8[iIndex++]="Enable";
textArray8[iIndex++]="Name of the File Server :";
textArray8[iIndex++]="File Server not found !";
textArray8[iIndex++]='<input type="button" value="Save & Restart" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.HTM
textArray9[iIndex++]="Workgroup";
textArray9[iIndex++]="Name :";
textArray9[iIndex++]="Shared Name";
textArray9[iIndex++]="Printer :";
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
textArray11[iIndex++]="Encryption algorithm :";
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
// End don't translate

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
//-----------------------------------------------------------------------------
function OnSiteSurvey()
{
	newwindow=window.open("WLANTMP0.HTM","","toolbar=0,location=0,directories=0,status=0,menubar=0,width=700,height=400,scrollbars=1");
	return false;
}

//-----------------------------------------------------------------------------
function validate_Preshared()
{	
	var f = document.myform;
	
	if( !CheckWPASharedKey(f) )
		return false;
	else
		return true;
}

//-----------------------------------------------------------------------------
function CheckWPASharedKey(f)
{
	var k = f.WPA_Pass.value;
	
	if( k == '' )
	{
		alert("Please input WPA Shared Key.");
		return false;
	}
	
	if( k.length == 64 )
	{
		if( !CheckHexKey(k) )
			return false;
	}
	else if( k.length < 8 || k.length > 63 )
	{
		alert("8 to 63 ASCII characters. Please try again.");
		return false;
	}
	
	return true;
}

//-----------------------------------------------------------------------------
function CheckHexKey(k)
{
	var iln, ch;

	for ( iln = 0; iln < k.length; iln++ )
	{
    	ch = k.charAt(iln).toLowerCase();
		
	  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') )
			continue;
	  	else 
		{
	    	alert("Invalid value " + ch + " in key " + k);
	    	return false;
	  	}
	}

	return true;
}

//-----------------------------------------------------------------------------
function SaveSettingCWLAN(szURL)
{
	var f = document.myform;
	var iln, ch;

	// Basic Settings
	// Wireless Mode
	
	// Advanced Settings
	// Security Mode
	if( f.security_mode[1].checked )
	{
		// WEP 64-bit or WEP 128-bit
		var hs;
	
		var iWEP_sel;			// Key Index
		var iWEP_type;			// WEP Encryption
		
		if (f.wep_sel[0].checked)
		{
			iWEP_sel = 0;
			
			if(f.wep_key1.value == '')
			{
				alert("WEP Key cannot be empty!");
				return false;
			}
		}
		if (f.wep_sel[1].checked)
		{
			iWEP_sel = 1;
			
			if(f.wep_key2.value == '')
			{
				alert("WEP Key cannot be empty!");
				return false;
			}
		}
		if (f.wep_sel[2].checked)
		{
			iWEP_sel = 2;
			
			if(f.wep_key3.value == '')
			{
				alert("WEP Key cannot be empty!");
				return false;
			}
		}
		if (f.wep_sel[3].checked)
		{
			iWEP_sel = 3;
			
			if(f.wep_key4.value == '')
			{
				alert("WEP Key cannot be empty!");
				return false;
			}
		}
		
		f.WLWEPKeySel.value = iWEP_sel;

		iWEP_type = f.wep_type.selectedIndex;
		// [0]: 64-bit 10 hex digits	[1]: 64-bit 5 characters
		// [2]: 128-bit 26 hex digits	[3]: 128-bit 13 characters

		// 64-bit hex or 128-bit hex
		if((iWEP_type == 0) || (iWEP_type == 2))
		{
			f.WLWEPFormat.value = 1;	// Hexadecimal

			switch(iWEP_sel)
			{
			case 0:			// WEP key 1
				hs = convertHexString (document.myform.wep_key1.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP key 1\" is not valid!");
					return false;
				}
				break;
			case 1:			// WEP key 2
				hs = convertHexString (document.myform.wep_key2.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP key 2\" is not valid!");
					return false;
				}
				break;
			case 2:			// WEP key 3
				hs = convertHexString (document.myform.wep_key3.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP key 3\" is not valid!");
					return false;
				}
				break;
			case 3:			// WEP key 4
				hs = convertHexString (document.myform.wep_key4.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP key 4\" is not valid!");
					return false;
				}
				break;
			default:		// WEP key 1
				hs = convertHexString (document.myform.wep_key1.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP key 1\" is not valid!");
					return false;
				}
			}
		}	// end of if((iWEP_type == 0) || (iWEP_type == 2))
		
		// 64-bit Alphanumeric or 128-bit Alphanumeric
		if((iWEP_type == 1) || (iWEP_type == 3))
		{
			f.WLWEPFormat.value = 0;		// Alphanumeric
			
			switch(iWEP_sel)
			{
			case 0:			// WEP key 1
				for ( iln = 0; iln < f.wep_key1.value.length; iln++ )
				{
			    	ch = f.wep_key1.value.charAt(iln);
					
				  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '!') || (ch == '?') )
				  		continue;
				  	else
				  	{
				  		alert ("\"WEP key 1\" is not valid!");
				  		return false;
				  	}
				}
				break;
			case 1:			// WEP key 2
				for ( iln = 0; iln < f.wep_key2.value.length; iln++ )
				{
			    	ch = f.wep_key2.value.charAt(iln);

				  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '!') || (ch == '?') )
				  		continue;
				  	else
				  	{
				  		alert ("\"WEP key 2\" is not valid!");
				  		return false;
				  	}
				}
				break;
			case 2:			// WEP key 3
				for ( iln = 0; iln < f.wep_key3.value.length; iln++ )
				{
			    	ch = f.wep_key3.value.charAt(iln);
					
				  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '!') || (ch == '?') )
				  		continue;
				  	else
				  	{
				  		alert ("\"WEP key 3\" is not valid!");
				  		return false;
				  	}
				}
				break;
			case 3:			// WEP key 4
				for ( iln = 0; iln < f.wep_key4.value.length; iln++ )
				{
			    	ch = f.wep_key4.value.charAt(iln);
					
				  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '!') || (ch == '?') )
				  		continue;
				  	else
				  	{
				  		alert ("\"WEP key 4\" is not valid!");
				  		return false;
				  	}
				}
				break;
			default:		// WEP key 1
				for ( iln = 0; iln < f.wep_key1.value.length; iln++ )
				{
			    	ch = f.wep_key1.value.charAt(iln);
					
				  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '!') || (ch == '?') )
				  		continue;
				  	else
				  	{
				  		alert ("\"WEP key 1\" is not valid!");
				  		return false;
				  	}
				}
			}
		}
		
		f.WLWEPType.value = 1;	// default 1: WEP 64-bit

		if((f.wep_type.options[0].selected) || (f.wep_type.options[1].selected))
		{
			f.WLWEPType.value = 1;	// WEP 64-bit

			f.WLWEPKey1.value = f.wep_key1.value;
			f.WLWEPKey2.value = f.wep_key2.value;
			f.WLWEPKey3.value = f.wep_key3.value;
			f.WLWEPKey4.value = f.wep_key4.value;
		}

		if((f.wep_type.options[2].selected) || (f.wep_type.options[3].selected))
		{
			// WEP 128-bit
			f.WLWEPType.value = 2;
			
			f.WLWEP128Key.value = f.wep_key1.value;
			f.WLWEP128Key2.value = f.wep_key2.value;
			f.WLWEP128Key3.value = f.wep_key3.value;
			f.WLWEP128Key4.value = f.wep_key4.value;
		}

		// Authentication
		if(f.wep_authmode[0].selected)
			f.WLAuthType.value = 1;		// Open System
		if(f.wep_authmode[1].selected)
			f.WLAuthType.value = 2;		// Shared Key
	}
	else if( f.security_mode[2].checked )
	{
		// WPA-PSK
		f.WLAuthType.value = 4;

		if(!validate_Preshared())
			return false;
	}
	else if( f.security_mode[3].checked )
	{
		// WPA2-PSK
		f.WLAuthType.value = 5;

		if(!validate_Preshared())
			return false;
	}
	else
	{
		// Disabled
		f.WLWEPType.value = 0;
		f.WLAuthType.value = 1;
	}

	document.forms[0].action=szURL;
	document.forms[0].submit();
	return false; 
}

function checkPreSharedKey(szURL)
{
	var theForm = document.forms[0];

	if( theForm.WLAuthType.value == "4" )
	{
		if( theForm.WPAPASS.value.length < 8 )
		{
			alert("8 to 63 characters!");
			return false;
		}
		else if( theForm.WPAPASS.value.indexOf(" ") >= 0 )
		{
			alert("Don't input the space character!");
			return false;
		}
		else
		{
			theForm.action=szURL;
			theForm.submit();
			return false;
		}
	}
	else
	{
		theForm.action=szURL;
		theForm.submit();
		return false;
	}
}

// out of English
