

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Korean
tabArray=['USB&nbsp;�����ͼ���','����','��ġ','��Ÿ','�����','���� �⺻��','�߿��� ���׷��̵�'];

	//upgrade.htm
headArray[iIndex++] ="<br>�� ���������� �����ͼ����� �߿�� ���׷��̵��Ͻ� �� �ֽ��ϴ�.<br>���� : �����Ͻñ� ���� �߿�� �ùٸ��� Ȯ���Ͻñ� �ٶ��ϴ�. ����ؾ��� �߿��� ������ �𸣽ø�, �ش� ������ �븮���� ������� ���Ǹ� �Ͻñ� �ٶ��ϴ�.";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "<b>���� �⺻��</b>�� Ŭ�� �� <b>Ȯ��</b>�� Ŭ���ϼż� �����ͼ����� ��� �⺻ ������ �ٽ� �ε��Ͻʽÿ�. ���! ���� ��� ������ �����˴ϴ�.";
textArray1[iIndex++] = "<b>�߿��� ���׷��̵�</b> �Ǹ� Ŭ���ϼż� �߿��� ���丮�� Ž���Ͻð� ���ο� �߿���� �����ͼ����� �ٽ� �ε��Ͻʽÿ�.";
//textArray1[iIndex++] = "You can still connect the print server with the current IP address after it restarts.";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;Ȯ��&nbsp;&nbsp;" onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="�߿��� ���׷��̵�";
textArray2[iIndex++]="�߿��� ���丮/���� ����:";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="�߿��� ���׷��̵�" onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="�� ���������� �����ͼ����� ������� �� �ֽ��ϴ�.<br>";
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>�����ͼ��� �����</B></FONT><br><br>������ �����ϰ� �����ͼ����� ������Ͻðڽ��ϱ�?<br><br>";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;Ȯ��&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

	//�����.htm
textArray4[iIndex++] = "������� ��";
textArray4[iIndex++] = "�����ͼ����� ������ϴ� ���� ��ٸ��ñ� �ٶ��ϴ�.";
iIndex = 0;
	//urestart.htm
textArray5[iIndex++] = "���׷��̵� �Ϸ�!";
textArray5[iIndex++] = "�߿��� ���׷��̵� ��, �����ͼ����� �ڵ����� ������Ͽ���, ��ø� ��ٷ��ֽñ� �ٶ��ϴ�. ";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "���� �⺻�� �ε��� ��";
textArray6[iIndex++] = "�⺻ ���� �ε� ��, �����ͼ����� �ڵ����� ������մϴ�.<BR><BR>�����ͼ����� ����۵Ǹ� ���� �������� �ǵ��ƿɴϴ�.";
iIndex = 0;

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=EUC-KR">');
}

// out of Korean
