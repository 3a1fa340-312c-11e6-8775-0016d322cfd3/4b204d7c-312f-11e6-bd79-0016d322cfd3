
//vaiable
tabindex = 0;
textindex = 0;
textindex10 = 0;		// CWLAN.htm Basic Settings
textindex11 = 0;		// CWLAN.htm Advanced Settings
textindex12 = 0;		// CWLAN.htm Site Survey
var iIndex = 0;

//Language : Simplified Chinese
//tabArray=['<IMG SRC=images/LOGO.gif>','״̬','����','����','����','ϵͳ','����','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];
tabArray=['�����ӡ������','״̬','����','����','����','ϵͳ','����','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>����Ļ�������Դ�ӡ���������л������ò�����<br>";
//cprinter.htm
//headArray[iIndex++] = "<BR>����Ļ�������������߹رմ�ӡ������ÿ���˿ڵ�˫���ӡ���ܡ�";
//cwlan.htm
headArray[iIndex++] = "<BR>����Ļ���������Ĵ�ӡ���������������á�";
//ctcpip.htm
headArray[iIndex++] = "<BR>����Ļ���������Ĵ�ӡ�������� TCP/IP ���á�";
//cnetware.htm
headArray[iIndex++] = "<BR>����Ļ���������Ĵ�ӡ�������� NetWare ���á�";
//capple.htm 
headArray[iIndex++] ="<BR>����Ļ���������Ĵ�ӡ�������� AppleTalk ���á�<br>";
//csnmp.htm
headArray[iIndex++] ="<BR>����Ļ���������Ĵ�ӡ�������� SNMP ���á�";
//csmp.htm
headArray[iIndex++] ="<BR>����Ļ���������Ĵ�ӡ��������΢�������ھӵĹ������á�";
iIndex = 0;

//csystem.htm
textArray0[iIndex++]="����֪ͨ����";
textArray0[iIndex++]="����֪ͨ:";
textArray0[iIndex++]="�ر�";
textArray0[iIndex++]="����";
textArray0[iIndex++]="SMTP ������ IP ��ַ:";
textArray0[iIndex++]="����Ա���ʵ�ַ:";
textArray0[iIndex++]="ϵͳ����";
textArray0[iIndex++]="�豸���� :";
textArray0[iIndex++]="ϵͳע�� :";
textArray0[iIndex++]="λ�� :";
textArray0[iIndex++]="����Ա����";
textArray0[iIndex++]="�û��� :";
textArray0[iIndex++]="admin";
textArray0[iIndex++]="���� :";
textArray0[iIndex++]="ȷ������ :";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="���沢����" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
//CPRINTER.htm
textArray1[iIndex++]="˫���ӡ��������";
textArray1[iIndex++]="��ӡ��������һ�˿� :";
textArray1[iIndex++]='<input type="button" value="���沢����" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
//CTCPIP.htm
textArray2[iIndex++]="TCP/IP ����";
textArray2[iIndex++]="�Զ���� TCP/IP ���� (ʹ�� DHCP/BOOTP)";
textArray2[iIndex++]="ʹ������ TCP/IP ����";
textArray2[iIndex++]="IP ��ַ :";
textArray2[iIndex++]="�������� :";
textArray2[iIndex++]="Ĭ������ :";
//crandvoo.htm
textArray2[iIndex++]="Rendezvous ����";
textArray2[iIndex++]="Rendezvous ���� :";
//textArray2[iIndex++]="Disable";
//textArray2[iIndex++]="Enable";
textArray2[iIndex++]="�������� :";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="���沢����" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="AppleTalk ����";
textArray3[iIndex++]="AppleTalk Zone ���� :";
textArray3[iIndex++]="�˿����� :";
textArray3[iIndex++]="��ӡ��";
textArray3[iIndex++]="��ӡ������ :";
textArray3[iIndex++]="���ݸ�ʽ :";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="���沢����" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
//CSNMP.htm
textArray4[iIndex++]="SNMP Community ����";
textArray4[iIndex++]="����ʹ�� HP WebJetAdmin :";
textArray4[iIndex++]="�ر�";
textArray4[iIndex++]="����";
textArray4[iIndex++]="Community 1 ���� :";
textArray4[iIndex++]="Ȩ�� :";
textArray4[iIndex++]="ֻ��";
textArray4[iIndex++]="��д";
textArray4[iIndex++]="Community 2 ���� :";
textArray4[iIndex++]="Ȩ�� :";
textArray4[iIndex++]="ֻ��";
textArray4[iIndex++]="��д";
textArray4[iIndex++]="SNMP Trap ����";
textArray4[iIndex++]="���� SNMP Traps :";
textArray4[iIndex++]="�ر�";
textArray4[iIndex++]="����";
textArray4[iIndex++]="ʹ����Ȩ�� Traps :";
textArray4[iIndex++]="�ر�";
textArray4[iIndex++]="����";
textArray4[iIndex++]="���� Traps ����һ�� IP ��ַ :";
textArray4[iIndex++]="���� Traps ���ڶ��� IP ��ַ :";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="���沢����" onClick="return SaveSetting(';
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
textArray5[iIndex++]='<INPUT TYPE=button VALUE=" �ر� " onClick="window.close()">';
iIndex = 0;
//browser.htm
//textArray6[iIndex++]="SSID";
//textArray6[iIndex++]="AP �� MAC ��ַ�� BSSID";
//textArray6[iIndex++]="Ƶ��(�ŵ�)";
//textArray6[iIndex++]="��������";
//textArray6[iIndex++]="��ȫ����";
//textArray6[iIndex++]="�ź��ܶ�";
// Translate                                  only "Rescan" is to be translated
//textArray6[iIndex++]='<INPUT TYPE=submit VALUE=" ˢ�� ">';
// Translate                                  only "Close" is to be translated
//textArray6[iIndex++]='<INPUT TYPE=button VALUE=" �ر� " onClick="window.close()">';
//iIndex = 0;
// End don't translate
// ERROR.htm
textArray7[iIndex++]="����";
textArray7[iIndex++]="��Ч IP ��ַ";
textArray7[iIndex++]="��Ч��������";
textArray7[iIndex++]="��Ч���ص�ַ";
textArray7[iIndex++]="Polling Time ֵ��Ч";
textArray7[iIndex++]="��Ч��ӡ��������";
textArray7[iIndex++]="��Ч�ļ���������";
textArray7[iIndex++]="δ�ҵ� DHCP/BOOTP ������";
textArray7[iIndex++]="��Ч SNMP Trap IP ��ַ";
textArray7[iIndex++]="���벻ƥ��";
textArray7[iIndex++]="���ʧЧ������ʧ��";
textArray7[iIndex++]="";
textArray7[iIndex++]="����";
iIndex = 0;

// CNETWARE.htm
textArray8[iIndex++]="��������";
textArray8[iIndex++]="��ӡ���������� :";
textArray8[iIndex++]="��ѯʱ�� :";
textArray8[iIndex++]="&nbsp;�� (3-29)";
textArray8[iIndex++]="��¼���� :";
textArray8[iIndex++]="NetWare NDS ����";
textArray8[iIndex++]="ʹ�� NDS ģʽ :";
textArray8[iIndex++]="�ر�";
textArray8[iIndex++]="����";
textArray8[iIndex++]="NDS Tree ���� :";
textArray8[iIndex++]="NDS Context ���� :";
textArray8[iIndex++]="NetWare Bindery ����";
textArray8[iIndex++]="ʹ�� Bindery ģʽ :";
textArray8[iIndex++]="�ر�";
textArray8[iIndex++]="����";
textArray8[iIndex++]="�ļ����������� :";
textArray8[iIndex++]="û���ҵ��ļ������� !";
textArray8[iIndex++]='<input type="button" value="���沢����" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.htm
textArray9[iIndex++]="������";
textArray9[iIndex++]="���� :";
textArray9[iIndex++]="�����ӡ������";
textArray9[iIndex++]="������ :";
textArray9[iIndex++]='<input type="button" value="���沢����" onClick="return CheckSMB(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;

// CWLAN.htm Basic Settings
textArray10[iIndex++]="��������";
textArray10[iIndex++]="�������� :";
textArray10[iIndex++]="SSID :";
textArray10[iIndex++]="Ƶ�� :";
//textArray10[iIndex++]="����������� :";
//textArray10[iIndex++]="1 - 11, ����, ���ô�, �ڿ���, �й�";
//textArray10[iIndex++]="1 - 13, ŷ�� (ETSI), ������������������";
//textArray10[iIndex++]="10 - 13, ����, �¼���";
//textArray10[iIndex++]="10 - 11, ������, ī����";
//textArray10[iIndex++]="���������������������ص��ص��ŷ��ɺ͹涨.";
//textArray10[iIndex++]="�뱣�����ò�������ӡ��������Ӧ�ô�����.";
//textArray10[iIndex++]='<input type="button" value="���沢����" onClick="return SaveSettingCWLAN(';
// Begin don't translate
//textArray10[iIndex++]="'RESTART.HTM');";
//textArray10[iIndex++]='">';
textArray10[iIndex++]="�������� :";
textArray10[iIndex++]="�Զ�";
textArray10[iIndex++]="����ģʽ :";
textArray10[iIndex++]="B/G/N ���ģʽ";
textArray10[iIndex++]="B/G ���ģʽ";
textArray10[iIndex++]="11B ģʽ";
textArray10[iIndex++]="11G ģʽ";
iIndex = 0;
// End don't translate

//CWLAN.htm Advanced Settings
textArray11[iIndex++]="�߼�����";
textArray11[iIndex++]="��ȫ����";
textArray11[iIndex++]="����";
textArray11[iIndex++]="WEP";
textArray11[iIndex++]="��Կѡ�� :";
textArray11[iIndex++]="��Կ��ʽ :";
textArray11[iIndex++]="64 λ (10 ��ʮ��������)";
textArray11[iIndex++]="64 λ (5 ���ַ�)";
textArray11[iIndex++]="128 λ (26 ��ʮ��������)";
textArray11[iIndex++]="128 λ (13 ���ַ�)";
textArray11[iIndex++]="��Կ 1 :";
textArray11[iIndex++]="��Կ 2 :";
textArray11[iIndex++]="��Կ 3 :";
textArray11[iIndex++]="��Կ 4 :";
textArray11[iIndex++]="��֤��ʽ :";
textArray11[iIndex++]="����ϵͳ";
textArray11[iIndex++]="������Կ";
//textArray11[iIndex++]="�Զ�";
textArray11[iIndex++]="WPA-PSK";
textArray11[iIndex++]="WPA2-PSK";
textArray11[iIndex++]="��Կ��ʽ :";
textArray11[iIndex++]="���簲ȫ��Կ :";
textArray11[iIndex++]="( ���� 8 �� 63 ���ַ� ( 0 �� 9, A �� Z ), ������ 64 ��ʮ���������� ( 0 �� 9, A �� F ) )";
// Translate                                  only "Save & Restart" is to be translated
textArray11[iIndex++]='<input type="button" value="���沢����" onClick="return SaveSettingCWLAN(';
// Begin don't translate
textArray11[iIndex++]="'RESTART.HTM');";
textArray11[iIndex++]='">';
iIndex = 0;
// End don't translate

//CWLAN.htm Site Survey
textArray12[iIndex++]="���������б�";
textArray12[iIndex++]="SSID";
textArray12[iIndex++]="MAC ��ַ";
textArray12[iIndex++]="Ƶ��";
textArray12[iIndex++]="����ģʽ";
textArray12[iIndex++]="��ȫ����";
textArray12[iIndex++]="�ź�ǿ��(dBm)";
textArray12[iIndex++]='<input name="refresh" onclick="return OnSiteSurvey()" type=button value=" ˢ�� " style="font-family: Verdana; font-size: 11px;">';
iIndex = 0;

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=gb2312">');
}

// functions
// CSYSTEM.HTM
function CheckPwd(szURL)
{
 	if(document.forms[0].SetupPWD.value != document.forms[0].ConfirmPWD.value && document.forms[0].SetupPWD.value != "ZO__I-SetupPassword" )
	{
		alert("����Ա�����ȷ�����벻ƥ�� !");
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
		alert("����! LPR �������Ʋ��ܿհ�!");
	 	return false; 
	}
	else
	{
		if ((document.forms[0].LPRQueue1.value == document.forms[0].LPRQueue2.value)
			||(document.forms[0].LPRQueue2.value == document.forms[0].LPRQueue3.value)
			||(document.forms[0].LPRQueue1.value == document.forms[0].LPRQueue3.value))
	 	{
	 		alert("����! LPR �������Ʋ����ظ�!");
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
		alert("����! ���������Ʋ��ܿհ�!");
		return false;
	}
	else
	{
		if(document.forms[0].SMBPrint1.value == '')
		{		
			alert("����! �����ӡ�����Ʋ��ܿհ�!");
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
		alert("������ WPA-PSK WPA2-PSK ���簲ȫ��Կ.");
		return false;
	}
	
	if( k.length == 64 )
	{
		if( !CheckHexKey(k) )
			return false;
	}
	else if( k.length < 8 || k.length > 63 )
	{
		alert("���� 8 �� 63 ���ַ� ( 0 �� 9, A �� Z ).");
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
		alert("������ WPA-PSK WPA2-PSK ���簲ȫ��Կ.");
		return false;
	}
	
	if( k.length == 64 )
	{
		if( !CheckHexKey(k) )
			return false;
	}
	else if( k.length < 8 || k.length > 63 )
	{
		alert("���� 8 �� 63 ���ַ� ( 0 �� 9, A �� Z ).");
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
	    	alert("��Ч�ַ� " + ch + " ��λ�ڵ� " + k);
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
				alert("WEP ��Կ���ܿհ�!");
				return false;
			}
		}
		if (f.wep_sel[1].checked)
		{
			iWEP_sel = 1;
			
			if(f.wep_key2.value == '')
			{
				alert("WEP ��Կ���ܿհ�!");
				return false;
			}
		}
		if (f.wep_sel[2].checked)
		{
			iWEP_sel = 2;
			
			if(f.wep_key3.value == '')
			{
				alert("WEP ��Կ���ܿհ�!");
				return false;
			}
		}
		if (f.wep_sel[3].checked)
		{
			iWEP_sel = 3;
			
			if(f.wep_key4.value == '')
			{
				alert("WEP ��Կ���ܿհ�!");
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
					alert ("\"WEP ��Կ 1\" ��Ч!");
					return false;
				}
				break;
			case 1:			// WEP key 2
				hs = convertHexString (document.myform.wep_key2.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP ��Կ 2\" ��Ч!");
					return false;
				}
				break;
			case 2:			// WEP key 3
				hs = convertHexString (document.myform.wep_key3.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP ��Կ 3\" ��Ч!");
					return false;
				}
				break;
			case 3:			// WEP key 4
				hs = convertHexString (document.myform.wep_key4.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP ��Կ 4\" ��Ч!");
					return false;
				}
				break;
			default:		// WEP key 1
				hs = convertHexString (document.myform.wep_key1.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP ��Կ 1\" ��Ч!");
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
				  		alert ("\"WEP ��Կ 1\" ��Ч!");
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
				  		alert ("\"WEP ��Կ 2\" ��Ч!");
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
				  		alert ("\"WEP ��Կ 3\" ��Ч!");
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
				  		alert ("\"WEP ��Կ 4\" ��Ч!");
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
				  		alert ("\"WEP ��Կ 1\" ��Ч!");
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
			alert("���� 8 �� 63 ���ַ� ( 0 �� 9, A �� Z )!");
			return false;
		}
		else if( theForm.WPAPASS.value.indexOf(" ") >= 0 )
		{
			alert("��Կ�м䲻���пհ�!");
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

// out of Simplified Chinese
