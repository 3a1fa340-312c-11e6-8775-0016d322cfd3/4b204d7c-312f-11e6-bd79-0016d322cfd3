

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Simplified Chinese
tabArray=['<IMG SRC=images/logo.jpg>','״̬','����','����','����','�ص�Ĭ��ֵ','�����̼�','����'];
//tabArray=['����/���� ˫ģ��ӡ������','״̬','����','����','����','�ص�Ĭ��ֵ','�����̼�','����'];

	//upgrade.htm
headArray[iIndex++] ="<br>����Ļ������������ӡ�������Ĺ̼���<br>ע��: ��������֮ǰ��ȷ�ϴ�ӡ�������Ĺ̼�����ȷ�ġ����������ȷ�ϣ����뵱�ع�Ӧ����ϵ��";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "��� ��<b>�ص�Ĭ��ֵ</b>�� Ȼ���� ��<b>ȷ��</b>�� �Իص�Ĭ��ֵ�� <BR><FONT CLASS=F1 COLOR=#FF3300><B>����! ��ӡ������������ֵ��ȫ���������</B></FONT><br><br>";
textArray1[iIndex++] = "�ص�Ĭ������, ������ TCP/IP ����. (�Ƽ�)";
textArray1[iIndex++] = "����������, ����Ȼ���õ�ǰ�� IP ��ַ����.";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;ȷ��&nbsp;&nbsp;" onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="�����̼�";
textArray2[iIndex++]="ѡ���ļ� :";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="�����̼�" onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="����Ļ����������������ӡ��������<br>";
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>����������ӡ������</B></FONT><br><br>��ȷ������Ҫ����������ӡ�������� ?<br><br>";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;ȷ��&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

	//restart.htm
textArray4[iIndex++] = "��������...";
textArray4[iIndex++] = "���Ե�";
iIndex = 0;
	//urestart.htm
textArray5[iIndex++] = "�����ɹ�!";
textArray5[iIndex++] = "�����ɹ��󣬴�ӡ�������Զ��������������Ե�";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "���¼��س���Ĭ��ֵ...";
textArray6[iIndex++] = "�ظ�����Ĭ��ֵ�󣬴˴�ӡ�������Զ�����������<BR><BR>����������, ��ӡ������������ص���ҳ.";
iIndex = 0;

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=gb2312">');
}

// out of Simplified Chinese
