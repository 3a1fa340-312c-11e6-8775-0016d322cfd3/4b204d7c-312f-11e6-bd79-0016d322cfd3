
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Deutsch
tabArray=['Druckservers','Status','Installation','Sonstiges','Neustart','System','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>Auf dieser Installationsseite können Sie die allgemeinen Systemeinstellungen des Druckservers konfigurieren.<br>";
//cprinter
headArray[iIndex++] = "<BR>Auf dieser Installationsseite können Sie die Bidirektionalität des Druckeranschlusses aktivieren/deaktivieren.<br>";
//ctcpip.htm
//headArray[iIndex++] = "<BR>This setup page allows you to configure wireless settings of the print server.";
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
textArray0[iIndex++]="Alarm per e-mail:";
textArray0[iIndex++]="deaktivieren";
textArray0[iIndex++]="aktivieren";
textArray0[iIndex++]="IP-Adresse des SMTP-Server:";
textArray0[iIndex++]="Administrator e-mail:";
textArray0[iIndex++]="Systemeinstellungen";
textArray0[iIndex++]="Name des Druckservers:";
textArray0[iIndex++]="Systemkontakt:";
textArray0[iIndex++]="Systemposition:";
textArray0[iIndex++]="Administratorkennwort";
textArray0[iIndex++]="Benutzername :";
textArray0[iIndex++]="admin";
textArray0[iIndex++]="Kennwort:";
textArray0[iIndex++]="Kennwort wiederholen:";
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
textArray2[iIndex++]="IP-Adresse:";
textArray2[iIndex++]="Subnet-Mask:";
textArray2[iIndex++]="Gateway:";
//crandvoo.htm
textArray2[iIndex++]="Rendezvous-Einstellungen";
textArray2[iIndex++]="Rendezvous-Einstellungen:";
textArray2[iIndex++]="deaktivieren";
textArray2[iIndex++]="aktivieren";
textArray2[iIndex++]="Dienstname:";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="Speichern und neu starten" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate

//CAPPLE.htm
textArray3[iIndex++]="AppleTalk-Einstellungen";
textArray3[iIndex++]="AppleTalk-Zone:";
textArray3[iIndex++]="Portname:";
textArray3[iIndex++]="Druckerkonfiguration";
textArray3[iIndex++]="Typ:";
textArray3[iIndex++]="Datenformat:";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="Speichern und neu starten" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
// CSNMP.HTM
textArray4[iIndex++]="SNMP-Community-Einstellungen";
textArray4[iIndex++]="Unterstützt HP WebJetAdmin:";
textArray4[iIndex++]="deaktivieren";
textArray4[iIndex++]="aktivieren";
textArray4[iIndex++]="SNMP-Community-Name 1:";
textArray4[iIndex++]="Privileg:";
textArray4[iIndex++]="Nur Lesen";
textArray4[iIndex++]="Lesen und Schreiben";
textArray4[iIndex++]="SNMP-Community-Name 2:";
textArray4[iIndex++]="Privileg:";
textArray4[iIndex++]="Nur Lesen";
textArray4[iIndex++]="Lesen und Schreiben";
textArray4[iIndex++]="SNMP-Trap-Einstellungen";
textArray4[iIndex++]="SNMP-Traps senden:";
textArray4[iIndex++]="deaktivieren";
textArray4[iIndex++]="aktivieren";
textArray4[iIndex++]="Authentifizierungs-Traps verwenden:";
textArray4[iIndex++]="deaktivieren";
textArray4[iIndex++]="aktivieren";
textArray4[iIndex++]="Trap-Adresse 1:";
textArray4[iIndex++]="Trap-Adresse 2:";
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
textArray7[iIndex++]="Dieses IP-Adresse ist falsch. Bitte versuchen Sie es erneut.";
textArray7[iIndex++]="Dieses Subnet-Mask ist falsch. Bitte versuchen Sie es erneut.";
textArray7[iIndex++]="Dieses Gateway ist falsch. Bitte versuchen Sie es erneut.";
textArray7[iIndex++]="Dieses Polling-Zeit ist falsch. Bitte versuchen Sie es erneut.";
textArray7[iIndex++]="Dieses Name des Druckservers ist falsch. Bitte versuchen Sie es erneut.";
textArray7[iIndex++]="Dieses Name des Dateiservers ist falsch. Bitte versuchen Sie es erneut.";
textArray7[iIndex++]="Kann keinen DHCP/BOOTP-Servers finden.";
textArray7[iIndex++]="Dieses SNMP-Trap-Adresse ist falsch. Bitte versuchen Sie es erneut.";
textArray7[iIndex++]="Setup password and confirmed do not match";
textArray7[iIndex++]="Wrong firmware or upgrade failed";
textArray7[iIndex++]="Failed in importing the CFG file";
textArray7[iIndex++]="";
textArray7[iIndex++]="Zurück";
iIndex = 0;

// CNETWARE.HTM
textArray8[iIndex++]="Allgemeine Einstellungen";
textArray8[iIndex++]="Name des Druckservers:";
textArray8[iIndex++]="Polling-Zeit:";
textArray8[iIndex++]="&nbsp;Sekunden (min.: 3, max.: 29 Sekunden)";
textArray8[iIndex++]="Logon-Kennwort:";
textArray8[iIndex++]="NDS-Einstellungen von NetWare";
textArray8[iIndex++]="NDS-Modus verwenden:";
textArray8[iIndex++]="deaktivieren";
textArray8[iIndex++]="aktivieren";
textArray8[iIndex++]="Name des NDS-Baums:";
textArray8[iIndex++]="Name des NDS-Kontexts:";
textArray8[iIndex++]="NetWare-Bindery-Einstellungen";
textArray8[iIndex++]="Bindery-Modus verwenden:";
textArray8[iIndex++]="deaktivieren";
textArray8[iIndex++]="aktivieren";
textArray8[iIndex++]="Name des Dateiservers:";
textArray8[iIndex++]="Kann keinen Dateiservers finden. !";
textArray8[iIndex++]='<input type="button" value="Speichern und neu starten" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.HTM
textArray9[iIndex++]="Workgroup";
textArray9[iIndex++]="Name:";
textArray9[iIndex++]="Gemeinsam genutzter Name";
textArray9[iIndex++]="Drucker:";
textArray9[iIndex++]='<input type="button" value="Speichern und neu starten" onClick="return SaveSetting(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;

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

// out of Deutsch
