
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Simplified Chinese
tabArray=['PU211S USB ��ӡ������',''];

//csystem.htm
textArray0[iIndex++]='<input type=button value="&nbsp;&nbsp;��������&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM'";
textArray0[iIndex++]='">';
textArray0[iIndex++]="��ӡ������¼";

textArray0[iIndex++]="ϵͳ��Ϣ";
textArray0[iIndex++]="�豸���� :";
textArray0[iIndex++]="ϵͳ����ʱ�� :";
textArray0[iIndex++]="�̼��汾 :";
textArray0[iIndex++]="MAC ��ַ :";

textArray0[iIndex++]="TCP/IP ����";
textArray0[iIndex++]="�Զ���� TCP/IP ���� (ʹ�� DHCP/BOOTP)";
textArray0[iIndex++]="ʹ������ TCP/IP ����";
textArray0[iIndex++]="IP ��ַ :";
textArray0[iIndex++]="�������� :";
textArray0[iIndex++]="Ĭ������ :";

textArray0[iIndex++]="����Ա����";
textArray0[iIndex++]="�û��� :";
textArray0[iIndex++]="���� :";
textArray0[iIndex++]="ȷ������ :";

textArray0[iIndex++]="��ӡ����Ϣ";
textArray0[iIndex++]="������ :";
textArray0[iIndex++]="�ͺ� :";
textArray0[iIndex++]="֧�ֵ����� :";
textArray0[iIndex++]="��ǰ״̬ :";
textArray0[iIndex++]="�ȴ���ӡ����....";
textArray0[iIndex++]="ȱֽ";
textArray0[iIndex++]="�ѻ�";
textArray0[iIndex++]="���ڴ�ӡ";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="���沢����" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';

textArray0[iIndex++]="�����̼�";
textArray0[iIndex++]="ѡ���ļ�:";
// Begin don't translate
textArray0[iIndex++]='<input type=button value="�����̼�" onClick="return WebUpgrade()">';

iIndex = 0;
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


// out of Simplified Chinese