
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Simplified Chinese
tabArray=['<IMG SRC=images/TL-PS110U-SC.jpg>','״̬','����','����','����','ϵͳ','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>����Ļ�������Դ�ӡ���������л������ò�����<br>";
//cprinter.htm
//headArray[iIndex++] = "<BR>����Ļ�������������߹رմ�ӡ������ÿ���˿ڵ�˫���ӡ���ܡ�";
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

// out of Simplified Chinese
