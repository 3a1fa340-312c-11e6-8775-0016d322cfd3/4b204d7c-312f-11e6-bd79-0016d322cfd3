
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : English
tabArray=['PU211S USB Print Server',''];

//csystem.htm
textArray0[iIndex++]='<input type=button value="&nbsp;&nbsp;Restart&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM'";
textArray0[iIndex++]='">';
textArray0[iIndex++]="View Job Log";

textArray0[iIndex++]="System Information";
textArray0[iIndex++]="Print Server Name :";
textArray0[iIndex++]="System Up Time :";
textArray0[iIndex++]="Firmware Version :";
textArray0[iIndex++]="MAC Address :";

textArray0[iIndex++]="TCP/IP Settings";
textArray0[iIndex++]="Obtain TCP/IP settings automatically (use DHCP/BOOTP)";
textArray0[iIndex++]="Use the following TCP/IP settings";
textArray0[iIndex++]="IP Address :";
textArray0[iIndex++]="Subnet Mask :";
textArray0[iIndex++]="Default Gateway :";

textArray0[iIndex++]="Administrator's Password";
textArray0[iIndex++]="Account :";
textArray0[iIndex++]="Password :";
textArray0[iIndex++]="Re-type Password :";

textArray0[iIndex++]="Printer Information";
textArray0[iIndex++]="Manufacturer :";
textArray0[iIndex++]="Model Number :";
textArray0[iIndex++]="Printing Language Supported :";
textArray0[iIndex++]="Current Status :";
textArray0[iIndex++]="Waiting for job";
textArray0[iIndex++]="Paper Out";
textArray0[iIndex++]="Offline";
textArray0[iIndex++]="Printing";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="Save & Restart" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';

textArray0[iIndex++]="Firmware Upgrade";
textArray0[iIndex++]="Select File:";
// Begin don't translate
textArray0[iIndex++]='<input type=button value="Upgrade" onClick="return WebUpgrade()">';

iIndex = 0;
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

// out of English
