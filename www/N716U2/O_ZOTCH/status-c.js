//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

tabArray=['PU201 ���� USB ��ӡ������','״̬','����','����','����','ϵͳ','��ӡ��','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
//Language : Simplified Chinese

//system.htm
headArray[iIndex++] = "<BR>����Ļ��ʾ��ӡ�������Ļ�����Ϣ��<BR>";
//printer.htm
headArray[iIndex++] = "<BR>����Ļ��ʾ���ӵ���ӡ��������ÿ̨��ӡ������ϸ��Ϣ��<BR>ע��: ������Ĵ�ӡ����֧��˫���ӡ���ܣ�һЩ��Ϣ��������ȷ����ʾ������";
//tcpip.htm
headArray[iIndex++] = "<BR>����Ļ��ʾ��ӡ��������ǰ TCP/IP ���á�<BR>";
//netware.htm
headArray[iIndex++] = "<BR>����Ļ��������ʾ��ӡ��������ǰ NetWare ���á�<BR>";
//apple.htm
headArray[iIndex++] = "<BR>����Ļ��ʾ��ӡ��������ǰ AppleTalk ���á�<BR>";
//snmp.htm
headArray[iIndex++] = "<BR>����Ļ��ʾ��ӡ��������ǰ SNMP ���á�<BR>";
//Smb.htm
headArray[iIndex++] = "<BR>����Ļ��ʾ��ӡ��������΢�������ھӵĹ������á�<BR>";
iIndex = 0;



//system.htm
textArray0[iIndex++]="ϵͳ��Ϣ";
textArray0[iIndex++]="�豸���� :";
textArray0[iIndex++]="ϵͳע�� :";
textArray0[iIndex++]="λ�� :";
textArray0[iIndex++]="ϵͳ����ʱ�� :";
textArray0[iIndex++]="�̼��汾 :";
textArray0[iIndex++]="MAC ��ַ :";
textArray0[iIndex++]="����֪ͨ :";
//PRINTJOB.htm
textArray0[iIndex++]="��ӡ����";
textArray0[iIndex++]="�������";
textArray0[iIndex++]="������";
textArray0[iIndex++]="����ʱ��";
textArray0[iIndex++]="Э��";
textArray0[iIndex++]="�˿�";
textArray0[iIndex++]="״̬";
textArray0[iIndex++]="��С(Bytes)";
textArray0[iIndex++]="��ӡ������¼";
iIndex = 0;

//Printer.htm
textArray1[iIndex++]="��ӡ����Ϣ";
textArray1[iIndex++]="������ :";
textArray1[iIndex++]="�ͺ� :";
textArray1[iIndex++]="֧�ֵ����� :";
textArray1[iIndex++]="��ǰ״̬ :";
iIndex = 0;

//NETWARE.htm
textArray2[iIndex++]="��������";
textArray2[iIndex++]="��ӡ���������� :";
textArray2[iIndex++]="��ѯʱ�� :";
textArray2[iIndex++]="NetWare NDS ����";
textArray2[iIndex++]="ʹ�� NDS ģʽ :";
textArray2[iIndex++]="NDS Tree ���� :";
textArray2[iIndex++]="NDS Context ���� :";
textArray2[iIndex++]="��ǰ״̬:";
textArray2[iIndex++]="NetWare Bindery ����";
textArray2[iIndex++]="ʹ�� Bindery ģʽ :";
textArray2[iIndex++]="�ļ����������� :";
textArray2[iIndex++]="��ǰ״̬ :";
iIndex = 0;
//tcpip.htm
textArray3[iIndex++]="TCP/IP ����";
textArray3[iIndex++]="ʹ�� DHCP/BOOTP :";
textArray3[iIndex++]="IP ��ַ :";
textArray3[iIndex++]="�������� :";
textArray3[iIndex++]="���� :";
//randvoo.htm
textArray3[iIndex++]="Rendezvous Settings";
textArray3[iIndex++]="Rendezvous Settings :";
//textArray3[iIndex++]="Disabled";
//textArray3[iIndex++]="Enabled";
textArray3[iIndex++]="Service Name :";
iIndex = 0;
//APPLE.htm
textArray4[iIndex++]="AppleTalk ����";
textArray4[iIndex++]="AppleTalk Zone ���� :";
textArray4[iIndex++]="��ӡ��";
textArray4[iIndex++]="�˿����� :";
textArray4[iIndex++]="��ӡ������ :";
textArray4[iIndex++]="���ݸ�ʽ :";
iIndex = 0;
//SNMP.htm
textArray5[iIndex++]="SNMP Community ����";
textArray5[iIndex++]="Community 1 ���� :";
textArray5[iIndex++]="Community 2 ���� :";
textArray5[iIndex++]="SNMP Trap ����";
textArray5[iIndex++]="���� SNMP Traps :";
textArray5[iIndex++]="ʹ����Ȩ�� Traps :";
textArray5[iIndex++]="���� Traps ����һ�� IP ��ַ :";
textArray5[iIndex++]="���� Traps ���ڶ��� IP ��ַ :";
iIndex = 0;

//JOBLOG.htm
// Translate                                  only "Refresh " is to be translated
textArray6[iIndex++]='<input type=button value=" ˢ�� " onClick="window.location.reload()">';
textArray6[iIndex++]="��ӡ����";
textArray6[iIndex++]="�������";
textArray6[iIndex++]="������";
textArray6[iIndex++]="����ʱ��";
textArray6[iIndex++]="Э��";
textArray6[iIndex++]="�˿�";
textArray6[iIndex++]="״̬";
textArray6[iIndex++]="��С(Bytes)";
// Translate                                  only "Close " is to be translated
textArray6[iIndex++]='<input type=button value=" �ر� " onClick="window.close()">';
iIndex = 0;

//SMB.htm
textArray7[iIndex++]="������";
textArray7[iIndex++]="���� :";
textArray7[iIndex++]="������ӡ������";
textArray7[iIndex++]="������ :";


// out of Simplified Chinese