
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : English
tabArray=['PU211S USB �L����A��',''];

//csystem.htm
textArray0[iIndex++]='<input type=button value="&nbsp;&nbsp;���s�Ұ�&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM'";
textArray0[iIndex++]='">';
textArray0[iIndex++]="�[�ݦC�L����";

textArray0[iIndex++]="�t�θ�T";
textArray0[iIndex++]="�˸m�W�� :";
textArray0[iIndex++]="�}���ɶ� :";
textArray0[iIndex++]="���骩�� :";
textArray0[iIndex++]="�����d��} :";

textArray0[iIndex++]="TCP/IP �]�w";
textArray0[iIndex++]="�۰ʨ��o IP ��} (�ϥ� DHCP/BOOTP)";
textArray0[iIndex++]="���w IP ��}";
textArray0[iIndex++]="IP ��} :";
textArray0[iIndex++]="�l�����B�n :";
textArray0[iIndex++]="�w�]�h�D�� :";

textArray0[iIndex++]="�t�κ޲z�̱K�X";
textArray0[iIndex++]="�b�� :";
textArray0[iIndex++]="�K�X :";
textArray0[iIndex++]="�K�X�T�{ :";

textArray0[iIndex++]="�L���";
textArray0[iIndex++]="�L����t�P :";
textArray0[iIndex++]="�L������� :";
textArray0[iIndex++]="�䴩���C�L�y�� :";
textArray0[iIndex++]="�ثe���A :";
textArray0[iIndex++]="�ݾ���";
textArray0[iIndex++]="�ʯ�";
textArray0[iIndex++]="���s���ΦL������u";
textArray0[iIndex++]="�C�L��";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="�x�s�í��s�Ұ�" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';

textArray0[iIndex++]="����ɯ�";
textArray0[iIndex++]="����ɮ�:";
// Begin don't translate
textArray0[iIndex++]='<input type=button value="����ɯ�" onClick="return WebUpgrade()">';

iIndex = 0;
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

// out of Chinese
