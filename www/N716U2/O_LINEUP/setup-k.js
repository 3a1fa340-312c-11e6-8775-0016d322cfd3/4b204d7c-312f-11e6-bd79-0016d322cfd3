
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Korean
//tabArray=['USB&nbsp;�����ͼ���','����','��ġ','��Ÿ','�����','�ý���','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];
tabArray=['USB&nbsp;�����ͼ���','����','��ġ','��Ÿ','�����','�ý���','����','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>�� ��ġ ���������� �����ͼ����� �Ϲ����� �ý��� ������ ������ �� �ֽ��ϴ�.<br>";
//ctcpip.htm
headArray[iIndex++] = "<BR>�� ��ġ ���������� �����ͼ����� TCP/IP������ ������ �� �ֽ��ϴ�.";
//cnetware.htm
headArray[iIndex++] = "<BR>�� ��ġ ���������� �����ͼ����� NetWare ����� ������ �� �ֽ��ϴ�.";
//capple.htm 
headArray[iIndex++] ="<BR>�� ��ġ ���������� �����ͼ����� AppleTalk ������ ������ �� �ֽ��ϴ�.<br>";
//csnmp.htm
headArray[iIndex++] ="<BR>�� ��ġ ���������� �����ͼ����� SNMP ������ ������ �� �ֽ��ϴ�.";
//csmp.htm
headArray[iIndex++] ="<BR>�� �������� ����ũ�μ���Ʈ Windows ��Ʈ��ũ������ ������ ���� ������ ǥ���մϴ�.";
iIndex = 0;

//csystem.htm
textArray0[iIndex++]="���� ���� �˸� ����";
textArray0[iIndex++]="���� ���� �˸� :";
textArray0[iIndex++]="������";
textArray0[iIndex++]="�����";
textArray0[iIndex++]="SMTP ���� IP �ּ� :";
textArray0[iIndex++]="������ ���� ���� �ּ� :";
textArray0[iIndex++]="�ý��� ����";
textArray0[iIndex++]="�����ͼ��� �̸� :";
textArray0[iIndex++]="�ý��� ���� :";

//textArray0[iIndex++]="<INPUT TYPE=TEXT NAME='SnmpSysContact' SIZE='20' MAXLENGTH='15' VALUE=";
//textArray0[iIndex++]='ZO__I-SnmpContact';
//textArray0[iIndex++]=">";

textArray0[iIndex++]="�ý��� ��ġ :";

//textArray0[iIndex++]="<INPUT TYPE=TEXT NAME='SnmpSysLocation' SIZE='20' MAXLENGTH='24' VALUE="
//textArray0[iIndex++]='ZO__I-SnmpLocation';
//textArray0[iIndex++]=">";

textArray0[iIndex++]="������ ��й�ȣ";
//textArray0[iIndex++]="Account :";
//textArray0[iIndex++]="admin";
textArray0[iIndex++]="��й�ȣ :";
textArray0[iIndex++]="��й�ȣ Ȯ�� :";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="���� & �����" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
//CPRINTER.htm
textArray1[iIndex++]="���� ��Ʈ - ����� ����";
textArray1[iIndex++]="�μ� ��Ʈ 1 :";
textArray1[iIndex++]='<input type="button" value="���� & �����" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
//CTCPIP.htm
textArray2[iIndex++]="TCP/IP ����";
textArray2[iIndex++]="TCP/IP ���� �ڵ� �Ҵ� (DHCP/BOOTP ���)";
textArray2[iIndex++]="���� TCP/IP ���� ���";
textArray2[iIndex++]="IP �ּ� :";
textArray2[iIndex++]="����� ����ũ :";
textArray2[iIndex++]="�⺻ ����� :";
//crandvoo.htm
textArray2[iIndex++]="������ ����";
textArray2[iIndex++]="������ ���� :";
//textArray2[iIndex++]="������";
//textArray2[iIndex++]="�����";
textArray2[iIndex++]="���� �̸� :";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="���� & �����" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="AppleTalk ����";
textArray3[iIndex++]="AppleTalk ���� :";
textArray3[iIndex++]="��Ʈ �̸� :";
textArray3[iIndex++]="������ ����";
textArray3[iIndex++]="���� :";
textArray3[iIndex++]="���̳ʸ� �������� :";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="���� & �����" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
//CSNMP.htm
textArray4[iIndex++]="SNMP Community ����";
textArray4[iIndex++]="HP WebJetAdmin ���� :";
textArray4[iIndex++]="������";
textArray4[iIndex++]="�����";
textArray4[iIndex++]="SNMP Community �̸� 1 :";
textArray4[iIndex++]="Ư�� :";
textArray4[iIndex++]="Read-Only";
textArray4[iIndex++]="Read-Write";
textArray4[iIndex++]="SNMP Community �̸� 2 :";
textArray4[iIndex++]="Ư�� :";
textArray4[iIndex++]="Read-Only";
textArray4[iIndex++]="Read-Write";
textArray4[iIndex++]="SNMP Ʈ�� ����";
textArray4[iIndex++]="SNMP Ʈ�� ���� :";
textArray4[iIndex++]="������";
textArray4[iIndex++]="�����";
textArray4[iIndex++]="���� Ʈ�� ��� :";
textArray4[iIndex++]="������";
textArray4[iIndex++]="�����";
textArray4[iIndex++]="Ʈ�� �ּ� 1 :";
textArray4[iIndex++]="Ʈ�� �ּ� 2 :";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="���� & �����" onClick="return SaveSetting(';
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
textArray5[iIndex++]='<INPUT TYPE=button value=" �ݱ� " onClick="window.close()">';
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
textArray7[iIndex++]="����";
textArray7[iIndex++]="�߸��� IP �ּ�";
textArray7[iIndex++]="�߸��� ����� ����ũ �ּ�";
textArray7[iIndex++]="�߸��� ����Ʈ���� �ּ�";
textArray7[iIndex++]="�߸��� ���� �ð���";
textArray7[iIndex++]="�߸��� �����ͼ��� �̸�";
textArray7[iIndex++]="�߸��� ���� ���� �̸�";
textArray7[iIndex++]="DHCP/BOOTP ������ ã�� �� �����ϴ�.";
textArray7[iIndex++]="�߸��� SNMP Ʈ�� IP �ּ�";
textArray7[iIndex++]="��ġ ��й�ȣ�� Ȯ�� ��й�ȣ�� ��ġ���� �ʽ��ϴ�.";
textArray7[iIndex++]="�߸��� �߿��� �Ǵ� ���׷��̵� ����";
textArray7[iIndex++]="Failed in importing the CFG file";
textArray7[iIndex++]="";
textArray7[iIndex++]="�ڷΰ���";
iIndex = 0;

// CNETWARE.htm
textArray8[iIndex++]="�Ϲ� ����";
textArray8[iIndex++]="�����ͼ��� �̸� :";
textArray8[iIndex++]="���� �ð� :";
textArray8[iIndex++]="&nbsp;�� (�ּ� : 3, �ִ� :  29��)";
textArray8[iIndex++]="�α׿� ��й�ȣ :";
textArray8[iIndex++]="NetWare NDS ����";
textArray8[iIndex++]="NDS ��� ��� :";
textArray8[iIndex++]="������";
textArray8[iIndex++]="�����";
textArray8[iIndex++]="NDS Ʈ�� �̸� :";
textArray8[iIndex++]="NDS ���ؽ�Ʈ �̸� :";
textArray8[iIndex++]="NetWare Bindery ����";
textArray8[iIndex++]="Bindery ��� ��� :";
textArray8[iIndex++]="������";
textArray8[iIndex++]="�����";
textArray8[iIndex++]="���� ���� �̸� :";
textArray8[iIndex++]="<option>Not found!</option>";
textArray8[iIndex++]='<input type="button" value="���� & �����" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.htm
textArray9[iIndex++]="�۾��׷�";
textArray9[iIndex++]="�̸�:";
textArray9[iIndex++]="���� �̸�";
textArray9[iIndex++]="������:";
textArray9[iIndex++]='<input type="button" value="���� & �����" onClick="return SaveSetting(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;

//CSERVICES.HTM
textArray10[iIndex++]="����";
textArray10[iIndex++]="Telnet:";
textArray10[iIndex++]="������";
textArray10[iIndex++]="�����";
textArray10[iIndex++]="HTTP:";
textArray10[iIndex++]="������";
textArray10[iIndex++]="�����";
textArray10[iIndex++]='<input type="button" value="���� & �����" onClick="return SaveServices(';
textArray10[iIndex++]="'RESTART.HTM');";
textArray10[iIndex++]='">';
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

	if(document.forms[0].SetupPWD.value.match('"')||document.forms[0].SetupPWD.value.match("'"))
	{
		alert("Administrator's Password not allowed below character !\n\'   \" ");
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
		if (document.forms[0].SMBPrint1.value == '')
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

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=EUC-KR">');
}

// out of Korean
