//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

tabArray=['USB �����ͼ���','����','��ġ','��Ÿ','�����','�ý���','������','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
//Language : Korean

//system.htm
headArray[iIndex++] = "<BR>�� �������� �����ͼ����� �Ϲ����� �ý��� ������ ǥ���մϴ�.<BR>";
//printer.htm
headArray[iIndex++] = "<BR>�� �������� ���� �����ͼ����� ����� �������� ������ ǥ���մϴ�.<BR>���� : �����Ϳ��� ����� ����� �������� �ʴ� ���, �Ϻ� ������ �ùٸ��� ǥ�õ��� ���� �� �ֽ��ϴ�.";
//tcpip.htm
headArray[iIndex++] = "<BR>�� �������� �����ͼ����� ���� TCP/IP ������ ǥ���մϴ�.<BR>";
//netware.htm
headArray[iIndex++] = "<BR>�� �������� �����ͼ����� ���� NetWare ������ ǥ���մϴ�.<BR>";
//apple.htm
headArray[iIndex++] = "<BR>�� �������� �����ͼ����� ���� AppleTalk ������ ǥ���մϴ�.<BR>";
//snmp.htm
headArray[iIndex++] = "<BR>�� �������� �����ͼ����� ���� SNMP ������ ǥ���մϴ�.<BR>";
//Smb.htm
headArray[iIndex++] = "<BR>�� �������� ����ũ�μ���Ʈ Windows ��Ʈ��ũ������ ������ ���� ������ ǥ���մϴ�.<BR>";
iIndex = 0;



//system.htm
textArray0[iIndex++]="�ý��� ����";
textArray0[iIndex++]="�����ͼ��� �̸� :";
textArray0[iIndex++]="�ý��� ���� :";
textArray0[iIndex++]="�ý��� ��ġ :";
textArray0[iIndex++]="�ý��� �ִ� �ð� :";
textArray0[iIndex++]="�߿��� ���� :";
textArray0[iIndex++]="MAC �ּ� :";
textArray0[iIndex++]="���� ���� �˸� :";
textArray0[iIndex++]="������";
textArray0[iIndex++]="�����";
//PRINTJOB.htm
textArray0[iIndex++]="�μ� �۾�";
textArray0[iIndex++]="�۾�";
textArray0[iIndex++]="�����";
textArray0[iIndex++]="��� �ð�";
textArray0[iIndex++]="��������";
textArray0[iIndex++]="��Ʈ";
textArray0[iIndex++]="����";
textArray0[iIndex++]="�μ� ����Ʈ";
textArray0[iIndex++]="�۾� �α� ����";
iIndex = 0;

//Printer.htm
textArray1[iIndex++]="������ ����";
textArray1[iIndex++]="������";
textArray1[iIndex++]="�� ��ȣ";
textArray1[iIndex++]="�μ� ���� ���";
textArray1[iIndex++]="���� ����";
textArray1[iIndex++]="�۾� ���";
textArray1[iIndex++]="���� ����";
textArray1[iIndex++]="��������";
textArray1[iIndex++]="�μ�";
iIndex = 0;

//NETWARE.htm
textArray2[iIndex++]="�Ϲ� ����";
textArray2[iIndex++]="�����ͼ��� �̸� :";
textArray2[iIndex++]="���� �ð� :";
textArray2[iIndex++]="��";
textArray2[iIndex++]="NetWare NDS ����";
textArray2[iIndex++]="NDS ��� ��� :";
textArray2[iIndex++]="������";
textArray2[iIndex++]="�����";
textArray2[iIndex++]="NDS Ʈ�� �̸� :";
textArray2[iIndex++]="NDS ���ؽ�Ʈ �̸� :";
textArray2[iIndex++]="���� ���� :";
textArray2[iIndex++]="���� ����";
textArray2[iIndex++]="�����";
textArray2[iIndex++]="NetWare Bindery ����";
textArray2[iIndex++]="Bindery ��� ��� :";
textArray2[iIndex++]="������";
textArray2[iIndex++]="�����";
textArray2[iIndex++]="���� ���� �̸� :";
textArray2[iIndex++]="���� ���� :";
textArray2[iIndex++]="���� ����";
textArray2[iIndex++]="�����";
iIndex = 0;
//tcpip.htm
textArray3[iIndex++]="TCP/IP ����";
textArray3[iIndex++]="DHCP/BOOTP ��� :";
textArray3[iIndex++]="������";
textArray3[iIndex++]="�����";
textArray3[iIndex++]="IP �ּ� :";
textArray3[iIndex++]="����� ����ũ :";
textArray3[iIndex++]="����Ʈ���� :";
//randvoo.htm
textArray3[iIndex++]="������ ����";
textArray3[iIndex++]="������ ���� :";
//textArray3[iIndex++]="������";
//textArray3[iIndex++]="�����";
textArray3[iIndex++]="���� �̸� :";
iIndex = 0;
//APPLE.htm
textArray4[iIndex++]="AppleTalk ����";
textArray4[iIndex++]="AppleTalk :";
textArray4[iIndex++]="������ ����";
textArray4[iIndex++]="��Ʈ �̸� :";
textArray4[iIndex++]="������ ���� :";
textArray4[iIndex++]="������ ���� :";
iIndex = 0;
//SNMP.htm
textArray5[iIndex++]="SNMP Community ����";
textArray5[iIndex++]="SNMP Community 1:";
textArray5[iIndex++]="�б� ����";
textArray5[iIndex++]="�б�/����";
textArray5[iIndex++]="SNMP Community 2:";
textArray5[iIndex++]="�б� ����";
textArray5[iIndex++]="�б�/����";
textArray5[iIndex++]="SNMP Ʈ�� ����";
textArray5[iIndex++]="SNMP Ʈ�� ���� :";
textArray5[iIndex++]="������";
textArray5[iIndex++]="�����";
textArray5[iIndex++]="���� Ʈ�� ��� :";
textArray5[iIndex++]="������";
textArray5[iIndex++]="�����";
textArray5[iIndex++]="Ʈ�� �ּ� 1 :";
textArray5[iIndex++]="Ʈ�� �ּ� 2 :";
iIndex = 0;

//JOBLOG.htm
// Translate                                  only "Refresh " is to be translated
textArray6[iIndex++]='<input type=button value=" ���ΰ�ħ " onClick="window.location.reload()">';
textArray6[iIndex++]="�μ� �۾�";
textArray6[iIndex++]="�۾�";
textArray6[iIndex++]="�����";
textArray6[iIndex++]="��� �ð�";
textArray6[iIndex++]="��������";
textArray6[iIndex++]="��Ʈ";
textArray6[iIndex++]="����";
textArray6[iIndex++]="�μ� ����Ʈ";
// Translate                                  only "Close " is to be translated
textArray6[iIndex++]='<input type=button value=" �ݱ� " onClick="window.close()">';
iIndex = 0;

//SMB.htm
textArray7[iIndex++]="�۾��׷�";
textArray7[iIndex++]="�̸�:";
textArray7[iIndex++]="���� �̸�";
textArray7[iIndex++]="������:";
iIndex = 0;

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=EUC-KR">');
}

// out of Korean
