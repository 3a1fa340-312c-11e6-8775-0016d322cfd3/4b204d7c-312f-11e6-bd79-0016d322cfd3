
//vaiable
tabindex = 0;
textindex = 0;
textindex10 = 0;		// CWLAN.htm Basic Settings
textindex11 = 0;		// CWLAN.htm Advanced Settings
textindex12 = 0;		// CWLAN.htm Site Survey
var iIndex = 0;

//Language : Traditional Chinese
tabArray=['<IMG SRC=images/logo.jpg>','Status','Setup','Misc','Restart','System','Wireless','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];
//tabArray=['�L�u/���u�������L����A��','���A','�]�w','�䥦','���s�Ұ�','�t��','�L�u','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>�����i�H���A�ק惡�L����A�����򥻳]�w�C<br>";
//cwlan.htm
headArray[iIndex++] = "<BR>�����i�H���A�ק惡�L����A���L�u�����]�w�C";
//ctcpip.htm
headArray[iIndex++] = "<BR>�����i�H���A�ק惡�L����A�� TCP/IP ���]�w�C";
//cnetware.htm
headArray[iIndex++] = "<BR>�����i�H���A�ק惡�L����A�� NetWare �������]�w���ءC";
//capple.htm 
headArray[iIndex++] = "<BR>�����i�H���A�ק惡�L����A�� AppleTalk �������]�w���ءC<br>";
//csnmp.htm
headArray[iIndex++] = "<BR>�����i�H���A�ק惡�L����A���� SNMP ���]�w�C";
//csmp.htm
headArray[iIndex++] = "<BR>�����i�H���A�ק惡�L����A�����L�n�����ھF�L������ɳ]�w�C";
iIndex = 0;

//csystem.htm
textArray0[iIndex++]="E-mail ĵ�ܳ]�w";
textArray0[iIndex++]="E-mail ĵ��:";
textArray0[iIndex++]="����";
textArray0[iIndex++]="�ҥ�";
textArray0[iIndex++]="�~�H�l����A�� IP �a�}:";
textArray0[iIndex++]="�޲z�� E-mail �H�c:";
textArray0[iIndex++]="�t�γ]�w";
textArray0[iIndex++]="�˸m�W�� :";
textArray0[iIndex++]="�s���H :";
textArray0[iIndex++]="�˸m��m :";
textArray0[iIndex++]="�t�κ޲z�̱K�X";
textArray0[iIndex++]="�޲z�̱b�� :";
textArray0[iIndex++]="admin";
textArray0[iIndex++]="�K�X :";
textArray0[iIndex++]="�K�X�T�{ :";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="�x�s�í��s�Ұ�" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
//CPRINTER.htm
textArray1[iIndex++]="����� - ���V�C�L�]�w";
textArray1[iIndex++]="�s���� 1 :";
textArray1[iIndex++]='<input type="button" value="�x�s�í��s�Ұ�" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
//CTCPIP.htm
textArray2[iIndex++]="TCP/IP �]�w";
textArray2[iIndex++]="�۰ʨ��o IP ��} (�ϥ� DHCP/BOOTP)";
textArray2[iIndex++]="���w IP ��}";
textArray2[iIndex++]="IP ��} :";
textArray2[iIndex++]="�l�����B�n :";
textArray2[iIndex++]="�w�]�h�D�� :";
//crandvoo.htm
textArray2[iIndex++]="Rendezvous �]�w";
textArray2[iIndex++]="Rendezvous �]�w :";
//textArray2[iIndex++]="����";
//textArray2[iIndex++]="�ҥ�";
textArray2[iIndex++]="�A�ȦW�� :";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="�x�s�í��s�Ұ�" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="AppleTalk �]�w";
textArray3[iIndex++]="AppleTalk �ϰ�W�� :";
textArray3[iIndex++]="�s����W�� :";
textArray3[iIndex++]="�s����";
textArray3[iIndex++]="�L����Φ� :";
textArray3[iIndex++]="��Ʈ榡 :";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="�x�s�í��s�Ұ�" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
//CSNMP.htm
textArray4[iIndex++]="SNMP �s��]�w";
textArray4[iIndex++]="�䴩 HP WebJetAdmin :";
textArray4[iIndex++]="����";
textArray4[iIndex++]="�ҥ�";
textArray4[iIndex++]="�s��W�� 1 :";
textArray4[iIndex++]="�s���v�� :";
textArray4[iIndex++]="�u��Ū";
textArray4[iIndex++]="Ū�g�ҥi";
textArray4[iIndex++]="�s��W�� 2 :";
textArray4[iIndex++]="�s���v�� :";
textArray4[iIndex++]="�u��Ū";
textArray4[iIndex++]="Ū�g�ҥi";
textArray4[iIndex++]="SNMP �����]�w";
textArray4[iIndex++]="�ϥγ����ɧ� :";
textArray4[iIndex++]="����";
textArray4[iIndex++]="�ҥ�";
textArray4[iIndex++]="�ǰe�T�{���� :";
textArray4[iIndex++]="����";
textArray4[iIndex++]="�ҥ�";
textArray4[iIndex++]="�����ؼ� IP ��} 1 :";
textArray4[iIndex++]="�����ؼ� IP ��} 2 :";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="�x�s�í��s�Ұ�" onClick="return SaveSetting(';
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
textArray5[iIndex++]='<INPUT TYPE=button VALUE=" ���� " onClick="window.close()">';
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
textArray7[iIndex++]="���~";
textArray7[iIndex++]="IP �a�}���~";
textArray7[iIndex++]="�l�����B�n���~";
textArray7[iIndex++]="�h�D�� IP �a�}���~";
textArray7[iIndex++]="���߶g���ȿ��~";
textArray7[iIndex++]="�L����A���W�ٿ��~";
textArray7[iIndex++]="NetWare �ɮצ��A���W�ٿ��~";
textArray7[iIndex++]="�䤣�� DHCP/BOOTP ���A��";
textArray7[iIndex++]="SNMP ���� IP �a�}���~";
textArray7[iIndex++]="�K�X���~";
textArray7[iIndex++]="���馳���D�Τɯť���";
textArray7[iIndex++]="";
textArray7[iIndex++]="�^��W�@��";
iIndex = 0;

// CNETWARE.htm
textArray8[iIndex++]="�򥻳]�w";
textArray8[iIndex++]="�L����A���W�� :";
textArray8[iIndex++]="���߮ɶ� :";
textArray8[iIndex++]="&nbsp;�� (�̤p: 3 ��, �̤j: 29 ��)";
textArray8[iIndex++]="�n�J NetWare ���K�X :";
textArray8[iIndex++]="NetWare NDS �]�w";
textArray8[iIndex++]="�ϥ� NDS �Ҧ� :";
textArray8[iIndex++]="����";
textArray8[iIndex++]="�ҥ�";
textArray8[iIndex++]="NDS Tree �W�� :";
textArray8[iIndex++]="NDS Context �W�� :";
textArray8[iIndex++]="NetWare Bindery �]�w";
textArray8[iIndex++]="�ϥ� Bindery �Ҧ� :";
textArray8[iIndex++]="����";
textArray8[iIndex++]="�ҥ�";
textArray8[iIndex++]="�ɮצ��A���W�� :";
textArray8[iIndex++]="<option>�䤣���ɮצ��A��!</option>";
textArray8[iIndex++]='<input type="button" value="�x�s�í��s�Ұ�" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.htm
textArray9[iIndex++]="�u�@�s��";
textArray9[iIndex++]="�W�� :";
textArray9[iIndex++]="�L����@�ΦW��";
textArray9[iIndex++]="�s���� :";
textArray9[iIndex++]='<input type="button" value="�x�s�í��s�Ұ�" onClick="return CheckSMB(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;

// CWLAN.htm Basic Settings
textArray10[iIndex++]="�򥻳]�w";
textArray10[iIndex++]="�������� :";
textArray10[iIndex++]="SSID :";
textArray10[iIndex++]="�W�D :";
//textArray10[iIndex++]="�L�u�a�� :";
//textArray10[iIndex++]="1 - 11, ����, �[���j, �Q�J��, ����";
//textArray10[iIndex++]="1 - 13, �ڬw (ETSI), �k��M��Z�����~";
//textArray10[iIndex++]="10 - 13, �k��, �s�[�Y";
//textArray10[iIndex++]="10 - 11, ��Z��, �����";
//textArray10[iIndex++]="�ܧ�L�u�a�ϳ]�m������u��a�q�H�k�O.";
//textArray10[iIndex++]="���x�s�í��s�ҰʥH�M�Φ����ܧ�.";
//textArray10[iIndex++]='<input type="button" value="�x�s�í��s�Ұ�" onClick="return SaveSettingCWLAN(';
// Begin don't translate
//textArray10[iIndex++]="'RESTART.HTM');";
//textArray10[iIndex++]='">';
textArray10[iIndex++]="�ǿ�t�v :";
textArray10[iIndex++]="�۰ʰ���";
textArray10[iIndex++]="�L�u�Ҧ� :";
textArray10[iIndex++]="�۰ʿ�� B/G";
textArray10[iIndex++]="�u�ϥ� B";
textArray10[iIndex++]="�u�ϥ� G";
textArray10[iIndex++]="�۰ʿ�� B/G/N";
iIndex = 0;
// End don't translate

//CWLAN.htm Advanced Settings
textArray11[iIndex++]="�i���]�w";
textArray11[iIndex++]="�w��������";
textArray11[iIndex++]="�������w����";
textArray11[iIndex++]="WEP";
textArray11[iIndex++]="���_���� :";
textArray11[iIndex++]="���_�[�K :";
textArray11[iIndex++]="64 �줸 (�Q���i����� 10)";
textArray11[iIndex++]="64 �줸 (�^�Ʀr���� 5)";
textArray11[iIndex++]="128 �줸 (�Q���i����� 26)";
textArray11[iIndex++]="128 �줸 (�^�Ʀr���� 13)";
textArray11[iIndex++]="���_ 1 :";
textArray11[iIndex++]="���_ 2 :";
textArray11[iIndex++]="���_ 3 :";
textArray11[iIndex++]="���_ 4 :";
textArray11[iIndex++]="�������� :";
textArray11[iIndex++]="�}�񦡨t��";
textArray11[iIndex++]="�@�ɪ��_";
//textArray11[iIndex++]="Auto";
textArray11[iIndex++]="WPA-PSK";
textArray11[iIndex++]="WPA2-PSK";
textArray11[iIndex++]="���_�[�K :";
textArray11[iIndex++]="�����w�����_ :";
textArray11[iIndex++]="�^�Ʀr���� 8 �� 63 (0 �� 9, A �� Z, a �� z), �ΤQ���i����� 64 (0 �� 9, A �� F)";
// Translate                                  only "Save & Restart" is to be translated
textArray11[iIndex++]='<input type="button" value="�x�s�í��s�Ұ�" onClick="return SaveSettingCWLAN(';
// Begin don't translate
textArray11[iIndex++]="'RESTART.HTM');";
textArray11[iIndex++]='">';
iIndex = 0;
// End don't translate

//CWLAN.htm Site Survey
textArray12[iIndex++]="�M��i�Ϊ��L�u����";
textArray12[iIndex++]="SSID";
textArray12[iIndex++]="MAC �a�}";
textArray12[iIndex++]="�W�D";
textArray12[iIndex++]="��������";
textArray12[iIndex++]="�w����";
textArray12[iIndex++]="�H���j��(dBm)";
textArray12[iIndex++]='<input name="refresh" onclick="return OnSiteSurvey()" type=button value=" ��s " style="font-family: Verdana; font-size: 11px;">';
iIndex = 0;

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=big5">');
}

// functions
// CSYSTEM.HTM
function CheckPwd(szURL)
{
 	if(document.forms[0].SetupPWD.value != document.forms[0].ConfirmPWD.value && document.forms[0].SetupPWD.value != "ZO__I-SetupPassword" )
	{
		alert("�t�κ޲z�̪��K�X�P�K�X�T�{���� !");
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
		alert("���~! LPR ��C�W�٤���ť�!");
	 	return false; 
	}
	else
	{
		if ((document.forms[0].LPRQueue1.value == document.forms[0].LPRQueue2.value)
			||(document.forms[0].LPRQueue2.value == document.forms[0].LPRQueue3.value)
			||(document.forms[0].LPRQueue1.value == document.forms[0].LPRQueue3.value))
	 	{
	 		alert("���~! LPR ��C�W�٤��୫��!");
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
		alert("���~! �u�@�s�զW�٤���ť�!");
		return false;
	}
	else
	{
		if(document.forms[0].SMBPrint1.value == '')
		{		
			alert("���~! SMB �@�ɦL����W�٤���ť�!"); 
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
		alert("�п�J WPA-PSK �� WPA2-PSK �@�ɪ��_.");
		return false;
	}
	
	if( k.length == 64 )
	{
		if( !CheckHexKey(k) )
			return false;
	}
	else if( k.length < 8 || k.length > 63 )
	{
		alert("�^�Ʀr���� 8 �� 63. �u���\ 0 �� 9, A �� Z, a �� z �o�Ǧr!");
		return false;
	}
	
	return true;
}

//-----------------------------------------------------------------------------
function CheckWPASharedKey(f)
{
	var k = f.WPA_Pass.value;
	
	if( k == '' )
	{
		alert("�п�J WPA-PSK �� WPA2-PSK �@�ɪ��_.");
		return false;
	}
	
	if( k.length == 64 )
	{
		if( !CheckHexKey(k) )
			return false;
	}
	else if( k.length < 8 || k.length > 63 )
	{
		alert("�^�Ʀr���� 8 �� 63. �u���\ 0 �� 9, A �� Z, a �� z �o�Ǧr!");
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
	    	alert("�z��J���������_�̪� " + ch + " ���O���Ħr, ���� " + k);
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
				alert("WEP ���_����ť�!");
				return false;
			}
		}
		if (f.wep_sel[1].checked)
		{
			iWEP_sel = 1;
			
			if(f.wep_key2.value == '')
			{
				alert("WEP ���_����ť�!");
				return false;
			}
		}
		if (f.wep_sel[2].checked)
		{
			iWEP_sel = 2;
			
			if(f.wep_key3.value == '')
			{
				alert("WEP ���_����ť�!");
				return false;
			}
		}
		if (f.wep_sel[3].checked)
		{
			iWEP_sel = 3;
			
			if(f.wep_key4.value == '')
			{
				alert("WEP ���_����ť�!");
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
					alert ("\"WEP ���_ 1\" �榡���~!");
					return false;
				}
				break;
			case 1:			// WEP key 2
				hs = convertHexString (document.myform.wep_key2.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP ���_ 2\" �榡���~!");
					return false;
				}
				break;
			case 2:			// WEP key 3
				hs = convertHexString (document.myform.wep_key3.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP ���_ 3\" �榡���~!");
					return false;
				}
				break;
			case 3:			// WEP key 4
				hs = convertHexString (document.myform.wep_key4.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP ���_ 4\" �榡���~!");
					return false;
				}
				break;
			default:		// WEP key 1
				hs = convertHexString (document.myform.wep_key1.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP ���_ 1\" �榡���~!");
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
				  		alert ("\"WEP ���_ 1\" �̦��L�Ħr, �u��g 0 �� 9, a �� z, A �� Z!");
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
				  		alert ("\"WEP ���_ 2\" �̦��L�Ħr, �u��g 0 �� 9, a �� z, A �� Z!");
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
				  		alert ("\"WEP ���_ 3\" �̦��L�Ħr, �u��g 0 �� 9, a �� z, A �� Z!");
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
				  		alert ("\"WEP ���_ 4\" �̦��L�Ħr, �u��g 0 �� 9, a �� z, A �� Z!");
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
				  		alert ("\"WEP ���_ 1\" �̦��L�Ħr, �u��g 0 �� 9, a �� z, A �� Z!");
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
			alert("�^�Ʀr���� 8 �� 63, �ΤQ���i����� 64!");
			return false;
		}
		else if( theForm.WPAPASS.value.indexOf(" ") >= 0 )
		{
			alert("�����w�����_�������঳�ť�!");
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

// out of Traditional Chinese
