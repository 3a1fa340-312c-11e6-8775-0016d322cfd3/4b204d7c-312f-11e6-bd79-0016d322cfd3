

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Russian
tabArray=['���������','������������','�������������','������������','��������� ��������� (�� ���������)','���������� ��'];

	//upgrade.htm
headArray[iIndex++] ="<br>��� �������� ��������� �������� ���������� �� ������� ������.<br>��������: ����� ��������� ����������, ���������, ��� �� ��������� ������� ������ ����������� ��. ���� �� �� ������, ����� ���� ����������� �� ��� ����� ������������, ���������� � ������ ����������� ��������� �������� ���������� ������������.";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "�������� �� ������ <b>��������� ���������</b>, � ����� <b>OK</b> ��� �������� ��������� �������� ������� ������. ��������������! ��� ������� ��������� ����� ������.";
textArray1[iIndex++] = "�������� �� ������ <b>���������� ��</b> ��� ��������� �������� ���������� ����������� �� � ������������ ������� ������ ����� ��������� ������ ����������� ��.";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;OK&nbsp;&nbsp;" onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="���������� ��";
textArray2[iIndex++]="������� ������� � ���� ���������� ����������� ��:";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="���������� ��" onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="��� �������� ������������ ��� ������������ ������� ������.<br>";
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>������������� ������ ������</B></FONT><br><br>��������� ������������ � ������������� ������ ������?<br><br>";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;OK&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

	//restart.htm
textArray4[iIndex++] = "������������...";
textArray4[iIndex++] = "��������� ���� ������ ��������������.";
iIndex = 0;
	//urestart.htm
textArray5[iIndex++] = "���������� ��������� �������!";
textArray5[iIndex++] = "����� ���������� ����������� �� ������ ������ ������������� ��������������. ���������.";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "�������� ��������� ������������ ...";
textArray6[iIndex++] = "����� �������� ��������� ������������, ������ ������ ������������� ��������������.<br><br>����� ������������ ��������� �������� Status (���������).";
iIndex = 0;
// out of Russian
