
//vaiable
tabindex = 0;
textindex = 0;
textindex10 = 0;		// CWLAN.htm Basic Settings
textindex11 = 0;		// CWLAN.htm Advanced Settings
textindex12 = 0;		// CWLAN.htm Site Survey
var iIndex = 0;

//Language : Deutsch
tabArray=['Druckservers','Status','Installation','Sonstiges','Neustart','System','Drahtlos','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>Auf dieser Installationsseite können Sie die allgemeinen Systemeinstellungen des Druckservers konfigurieren.<br>";
//cprinter
//headArray[iIndex++] = "<BR>Auf dieser Installationsseite können Sie die Bidirektionalität des Druckeranschlusses aktivieren/deaktivieren.<br>";
//cwlan.htm
headArray[iIndex++] = "<BR>Auf dieser Installationsseite können Sie die Drahtlos-Einstellungen des Druckservers konfigurieren.";
//ctcpip.htm
headArray[iIndex++] = "<BR>Auf dieser Installationsseite können Sie die TCP/IP-Einstellungen des Druckservers konfigurieren.";
//cnetware.htm
headArray[iIndex++] = "<BR>Auf dieser Installationsseite können Sie die NetWare-Funktion des Druckservers konfigurieren.";
//capple.htm 
headArray[iIndex++] ="<BR>Auf dieser Installationsseite können Sie die AppleTalk-Einstellungen des Druckservers konfigurieren.<br>";
//csnmp.htm
headArray[iIndex++] ="<BR>Auf dieser Installationsseite können Sie die SNMP-Einstellungen des Druckservers konfigurieren.";
//csmp.htm
headArray[iIndex++] ="<BR>Auf dieser Seite werden die Einstellungen für die gemeinsame Druckerverwendung bei Microsoft Windows-Netzwerken angezeigt.";
iIndex = 0;

// CSYSTEM.HTM
textArray0[iIndex++]="Alarm per e-mail Einstellungen";
textArray0[iIndex++]="Alarm per e-mail :";
textArray0[iIndex++]="deaktivieren";
textArray0[iIndex++]="aktivieren";
textArray0[iIndex++]="IP-Adresse des SMTP-Server :";
textArray0[iIndex++]="Administrator e-mail :";
textArray0[iIndex++]="Systemeinstellungen";
textArray0[iIndex++]="Name des Druckservers :";
textArray0[iIndex++]="Systemkontakt :";
textArray0[iIndex++]="Systemposition :";
textArray0[iIndex++]="Administratorkennwort";
textArray0[iIndex++]="Benutzername :";
textArray0[iIndex++]="admin";
textArray0[iIndex++]="Kennwort :";
textArray0[iIndex++]="Kennwort wiederholen :";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="Speichern und neu starten" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
// CPRINTER.HTM
textArray1[iIndex++]="Parallelanschluss - Bidirektionale Einstellungen";
textArray1[iIndex++]="Druckeranschluss:";
textArray1[iIndex++]="Deaktivieren";
textArray1[iIndex++]="Auto";
textArray1[iIndex++]="Druckgeschwindigkeit :";
textArray1[iIndex++]="schnell";
textArray1[iIndex++]="mittel";
textArray1[iIndex++]="langsam";
textArray1[iIndex++]="Für Laserdrucker, wird schnell empfohlen.";
textArray1[iIndex++]="Für Nadeldrucker, wird langsam empfehlen.";
//textArray1[iIndex++]="LPR Queue Name :";
//textArray1[iIndex++]="(Max 12 alphanumeric characters without any blank characters)";
textArray1[iIndex++]='<input type="button" value="Speichern und neu starten" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
// CTCPIP.HTM
textArray2[iIndex++]="TCP/IP-Einstellungen";
textArray2[iIndex++]="TCP/IP-Einstellungen automatisch laden (DHCP/BOOTP verwenden)";
textArray2[iIndex++]="Die folgenden TCP/IP-Einstellungen verwenden";
textArray2[iIndex++]="IP-Adresse :";
textArray2[iIndex++]="Subnet-Mask :";
textArray2[iIndex++]="Gateway :";
//crandvoo.htm
textArray2[iIndex++]="Rendezvous-Einstellungen";
textArray2[iIndex++]="Rendezvous-Einstellungen :";
textArray2[iIndex++]="deaktivieren";
textArray2[iIndex++]="aktivieren";
textArray2[iIndex++]="Dienstname :";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="Speichern und neu starten" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate

//CAPPLE.htm
textArray3[iIndex++]="AppleTalk-Einstellungen";
textArray3[iIndex++]="AppleTalk-Zone :";
textArray3[iIndex++]="Portname :";
textArray3[iIndex++]="Druckerkonfiguration";
textArray3[iIndex++]="Typ :";
textArray3[iIndex++]="Datenformat :";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="Speichern und neu starten" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
// CSNMP.HTM
textArray4[iIndex++]="SNMP-Community-Einstellungen";
textArray4[iIndex++]="Unterstützt HP WebJetAdmin :";
textArray4[iIndex++]="deaktivieren";
textArray4[iIndex++]="aktivieren";
textArray4[iIndex++]="SNMP-Community-Name 1 :";
textArray4[iIndex++]="Privileg:";
textArray4[iIndex++]="Nur Lesen";
textArray4[iIndex++]="Lesen und Schreiben";
textArray4[iIndex++]="SNMP-Community-Name 2 :";
textArray4[iIndex++]="Privileg:";
textArray4[iIndex++]="Nur Lesen";
textArray4[iIndex++]="Lesen und Schreiben";
textArray4[iIndex++]="SNMP-Trap-Einstellungen";
textArray4[iIndex++]="SNMP-Traps senden :";
textArray4[iIndex++]="deaktivieren";
textArray4[iIndex++]="aktivieren";
textArray4[iIndex++]="Authentifizierungs-Traps verwenden :";
textArray4[iIndex++]="deaktivieren";
textArray4[iIndex++]="aktivieren";
textArray4[iIndex++]="Trap-Adresse 1 :";
textArray4[iIndex++]="Trap-Adresse 2 :";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="Speichern und neu starten" onClick="return SaveSetting(';
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
textArray7[iIndex++]="FEHLER";
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
textArray7[iIndex++]="Zurück";
iIndex = 0;

// CNETWARE.HTM
textArray8[iIndex++]="Allgemeine Einstellungen";
textArray8[iIndex++]="Name des Druckservers :";
textArray8[iIndex++]="Polling-Zeit :";
textArray8[iIndex++]="&nbsp;Sekunden (min.: 3, max.: 29 Sekunden)";
textArray8[iIndex++]="Logon-Kennwort :";
textArray8[iIndex++]="NDS-Einstellungen von NetWare";
textArray8[iIndex++]="NDS-Modus verwenden :";
textArray8[iIndex++]="deaktivieren";
textArray8[iIndex++]="aktivieren";
textArray8[iIndex++]="Name des NDS-Baums :";
textArray8[iIndex++]="Name des NDS-Kontexts :";
textArray8[iIndex++]="NetWare-Bindery-Einstellungen";
textArray8[iIndex++]="Bindery-Modus verwenden :";
textArray8[iIndex++]="deaktivieren";
textArray8[iIndex++]="aktivieren";
textArray8[iIndex++]="Name des Dateiservers :";
textArray8[iIndex++]="Kann keinen Dateiservers finden. !";
textArray8[iIndex++]='<input type="button" value="Speichern und neu starten" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.HTM
textArray9[iIndex++]="Workgroup";
textArray9[iIndex++]="Name :";
textArray9[iIndex++]="Gemeinsam genutzter Name";
textArray9[iIndex++]="Drucker :";
textArray9[iIndex++]='<input type="button" value="Speichern und neu starten" onClick="return CheckSMB(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;

// CWLAN.htm Basic Settings
textArray10[iIndex++]="Grundeinstellungen";
textArray10[iIndex++]="Modi betrieben werden :";
textArray10[iIndex++]="SSID :";
textArray10[iIndex++]="Kanal :";
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
textArray10[iIndex++]="Datenübertragungsraten :";
textArray10[iIndex++]="Auto";
textArray10[iIndex++]="Norm (Standard) :";
textArray10[iIndex++]="Gemischter 802.11bg";
textArray10[iIndex++]="Nur 802.11b";
textArray10[iIndex++]="Nur 802.11g";
textArray10[iIndex++]="Gemischter 802.11bgn";
iIndex = 0;
// End don't translate

//CWLAN.htm Advanced Settings
textArray11[iIndex++]="Erweiterte Einstellungen";
textArray11[iIndex++]="Verschlüsselung";
textArray11[iIndex++]="Deaktivieren";
textArray11[iIndex++]="WEP";
textArray11[iIndex++]="Schlüssel-index :";
textArray11[iIndex++]="Typ (Schlüssellängen) :";
textArray11[iIndex++]="64-bit (10 Hexadezimalzeichen ein)";
textArray11[iIndex++]="64-bit (5 Zeichen)";
textArray11[iIndex++]="128-bit (26 Hexadezimalzeichen ein)";
textArray11[iIndex++]="128-bit (13 Zeichen)";
textArray11[iIndex++]="Auto";
textArray11[iIndex++]="Schlüssel 1 :";
textArray11[iIndex++]="Schlüssel 2 :";
textArray11[iIndex++]="Schlüssel 3 :";
textArray11[iIndex++]="Schlüssel 4 :";
textArray11[iIndex++]="Authentifizierung :";
textArray11[iIndex++]="Open System";
textArray11[iIndex++]="Shared Key";
//textArray11[iIndex++]="Auto";
textArray11[iIndex++]="WPA-PSK";
textArray11[iIndex++]="WPA2-PSK";
textArray11[iIndex++]="Verschlüsselungsalgorithmus";
textArray11[iIndex++]="Netzwerk-schlüssel :";
textArray11[iIndex++]="( 8 bis 63 Zeichen ( ASCII ), oder 64 Hexadezimalzeichen ein ( 0 bis 9, A bis F ) )";
// Translate                                  only "Save & Restart" is to be translated
textArray11[iIndex++]='<input type="button" value="Speichern und neu starten" onClick="return SaveSettingCWLAN(';
// Begin don't translate
textArray11[iIndex++]="'RESTART.HTM');";
textArray11[iIndex++]='">';
iIndex = 0;
// End don't translate

//CWLAN.htm Site Survey
textArray12[iIndex++]="Site Survey";
textArray12[iIndex++]="SSID";
textArray12[iIndex++]="MAC-Adresse";
textArray12[iIndex++]="Kanal";
textArray12[iIndex++]="Modus";
textArray12[iIndex++]="Verschlüsselung";
textArray12[iIndex++]="Signalintensität(dBm)";
textArray12[iIndex++]='<input name="refresh" onclick="return OnSiteSurvey()" type=button value=" Schließen " style="font-family: Verdana; font-size: 11px;">';
iIndex = 0;
// End don't translate

// functions
// CSYSTEM.HTM
function CheckPwd(szURL)
{
 	if(document.forms[0].SetupPWD.value != document.forms[0].ConfirmPWD.value && document.forms[0].SetupPWD.value != "ZO__I-SetupPassword" )
	{
		alert("Administrator Passwort und Abfrage passen nicht zueinander.");
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
		alert("FEHLER! Das Workgroupfeld darf nicht leer sein.");
		return false;
	}
	else
	{
		if(document.forms[0].SMBPrint1.value == '')
		{		
			alert("FEHLER! Das Gemeinsam genutzter Name darf nicht leer sein.");
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
		alert("Dieses netzwerk-schlüssel ist falsch. Bitte versuchen Sie es erneut.");
		return false;
	}
	
	if( k.length == 64 )
	{
		if( !CheckHexKey(k) )
			return false;
	}
	else if( k.length < 8 || k.length > 63 )
	{
		alert("8 bis 63 Zeichen. Bitte versuchen Sie es erneut.");
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
	    	alert("Falscher zeichen " + ch + " in schlüssel " + k);
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
		var iKey_len;
		
		if (f.wep_sel[0].checked)
		{
			iWEP_sel = 0;
			
			if(f.wep_key1.value == '')
			{
				alert("Dieses WEP-schlüssel 1 ist falsch. Bitte versuchen Sie es erneut.");
				return false;
			}
			
			iKey_len = f.wep_key1.value.length;
		}
		if (f.wep_sel[1].checked)
		{
			iWEP_sel = 1;
			
			if(f.wep_key2.value == '')
			{
				alert("Dieses WEP-schlüssel 2 ist falsch. Bitte versuchen Sie es erneut.");
				return false;
			}
			
			iKey_len = f.wep_key2.value.length;
		}
		if (f.wep_sel[2].checked)
		{
			iWEP_sel = 2;
			
			if(f.wep_key3.value == '')
			{
				alert("Dieses netzwerk-schlüssel 3 ist falsch. Bitte versuchen Sie es erneut.");
				return false;
			}
			
			iKey_len = f.wep_key3.value.length;
		}
		if (f.wep_sel[3].checked)
		{
			iWEP_sel = 3;
			
			if(f.wep_key4.value == '')
			{
				alert("Dieses netzwerk-schlüssel 4 ist falsch. Bitte versuchen Sie es erneut.");
				return false;
			}
			
			iKey_len = f.wep_key4.value.length;
		}
		
		iWEP_type = f.wep_type.selectedIndex;
		if(iWEP_type == 4)
		{
				if(iKey_len==10)
				{
						iWEP_type = 0;
						f.wep_type.options[0].selected = true;
						f.wep_type.options[1].selected = false;
						f.wep_type.options[2].selected = false;
						f.wep_type.options[3].selected = false;
						f.wep_type.options[4].selected = false;
				}
				else if(iKey_len==5)
				{
						iWEP_type = 1;
						f.wep_type.options[0].selected = false;
						f.wep_type.options[1].selected = true;
						f.wep_type.options[2].selected = false;
						f.wep_type.options[3].selected = false;
						f.wep_type.options[4].selected = false;						
				}
				else if(iKey_len==26)
				{
						iWEP_type = 2;
						f.wep_type.options[0].selected = false;
						f.wep_type.options[1].selected = false;
						f.wep_type.options[2].selected = true;
						f.wep_type.options[3].selected = false;
						f.wep_type.options[4].selected = false;							
				}
				else if(iKey_len==13)
				{
						iWEP_type = 3;
						f.wep_type.options[0].selected = false;
						f.wep_type.options[1].selected = false;
						f.wep_type.options[2].selected = false;
						f.wep_type.options[3].selected = true;
						f.wep_type.options[4].selected = false;							
				}
				
				f.wep_type.selectedIndex = iWEP_type;		
		}
		
		f.WLWEPKeySel.value = iWEP_sel;

		// iWEP_type = f.wep_type.selectedIndex;
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
					alert ("WEP-schlüssel 1 ist falsch.");
					return false;
				}
				break;
			case 1:			// WEP key 2
				hs = convertHexString (document.myform.wep_key2.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("WEP-schlüssel 2 ist falsch.");
					return false;
				}
				break;
			case 2:			// WEP key 3
				hs = convertHexString (document.myform.wep_key3.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("WEP-schlüssel 3 ist falsch.");
					return false;
				}
				break;
			case 3:			// WEP key 4
				hs = convertHexString (document.myform.wep_key4.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("WEP-schlüssel 4 ist falsch.");
					return false;
				}
				break;
			default:		// WEP key 1
				hs = convertHexString (document.myform.wep_key1.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("WEP-schlüssel 1 ist falsch.");
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
				  		alert ("WEP-schlüssel 1 ist falsch.");
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
				  		alert ("WEP-schlüssel 2 ist falsch.");
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
				  		alert ("WEP-schlüssel 3 ist falsch.");
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
				  		alert ("WEP-schlüssel 4 ist falsch.");
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
				  		alert ("WEP-schlüssel 1 ist falsch.");
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
			alert("8 bis 63 Zeichen. Bitte versuchen Sie es erneut.");
			return false;
		}
		else if( theForm.WPAPASS.value.indexOf(" ") >= 0 )
		{
			alert("Dieses netzwerk-schlüssel ist falsch. Bitte versuchen Sie es erneut.");
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

// out of Deutsch
