//vaiable
menuindex = 0;
tabindex = 0;
textindex = 0;

var iIndex = 0;

menuArray=['NP302','ϵͳ��Ϣ','ϵͳ����','��ӡ����','��������','ϵͳ����','�ۺ����'];
tabArray=['ϵͳ','��ӡ��','������ IP','NetWare','AppleTalk','SNMP','SMB'];
//Language : Simplified Chinese

//system.htm 0
headArray[iIndex++] = "ϵͳ��Ϣ";
//system.htm
headArray[iIndex++] = "��ҳ����ʾ��ӡ�������Ļ�����Ϣ��<BR>";
//printer.htm 0
headArray[iIndex++] = "��ӡ��";
//printer.htm
headArray[iIndex++] = "��ʾ���ӵ���ӡ��������ĳ̨��ӡ������Ϣ��<BR>ע��: ������Ĵ�ӡ����֧�� bi-directional ���ܣ�ĳЩ��Ϣ��������ʾ��";
//tcpip.htm 0
headArray[iIndex++] = "��ӡ������ IP";
//tcpip.htm
headArray[iIndex++] = "��ʾ��ӡ������ IP ���õ������Ϣ��<BR>";
//netware.htm 0
headArray[iIndex++] = "NetWare";
//netware.htm
headArray[iIndex++] = "��ʾ��ӡ������ NetWare ���õ������Ϣ��<BR>";
//apple.htm 0
headArray[iIndex++] = "AppleTalk";
//apple.htm
headArray[iIndex++] = "��ʾ��ӡ������ AppleTalk ���õ������Ϣ��<BR>";
//snmp.htm 0
headArray[iIndex++] = "SNMP";
//snmp.htm
headArray[iIndex++] = "��ʾ��ӡ������ SNMP ���õ������Ϣ��<BR>";
//smb.htm 0
headArray[iIndex++] = "SMB";
//Smb.htm
headArray[iIndex++] = "��ʾ��ӡ���������ϵĹ������õ������Ϣ��<BR>";
iIndex = 0;



//system.htm
textArray0[iIndex++]="ϵͳ��Ϣ";
textArray0[iIndex++]="��ӡ���������� :";
textArray0[iIndex++]="������ע�� :";
textArray0[iIndex++]="������λ�� :";
textArray0[iIndex++]="ϵͳ����ʱ�� :";
textArray0[iIndex++]="����汾 :";
textArray0[iIndex++]="MAC ��ַ :";
textArray0[iIndex++]="E-mail ֪ͨ :";
//textArray0[iIndex++]="�ر�";
//textArray0[iIndex++]="����";
//PRINTJOB.htm
textArray0[iIndex++]="��ӡ����";
textArray0[iIndex++]="����";
textArray0[iIndex++]="�û�";
textArray0[iIndex++]="����ִ��ʱ��";
textArray0[iIndex++]="Э��";
textArray0[iIndex++]="�˿�";
textArray0[iIndex++]="״̬";
textArray0[iIndex++]="��ӡ�ֽ���";
textArray0[iIndex++]="��ӡ������¼";
iIndex = 0;

//Printer.htm
textArray1[iIndex++]="��ӡ����Ϣ";
textArray1[iIndex++]="�������� :";
textArray1[iIndex++]="�ͺ� :";
textArray1[iIndex++]="֧������ :";
textArray1[iIndex++]="��ǰ״̬ :";
//textArray1[iIndex++]="�ȴ���ӡ����....";
//textArray1[iIndex++]="ȱֽ";
//textArray1[iIndex++]="�ѻ�";
//textArray1[iIndex++]="���ڴ�ӡ";
iIndex = 0;

//NETWARE.htm
textArray2[iIndex++]="��������";
textArray2[iIndex++]="��ӡ���������� :";
textArray2[iIndex++]="��ѯʱ�� :";
//textArray2[iIndex++]="��";
textArray2[iIndex++]="NetWare NDS ����";
textArray2[iIndex++]="ʹ�� NDS ģʽ :";
//textArray2[iIndex++]="�ر�";
//textArray2[iIndex++]="����";
textArray2[iIndex++]="NDS Tree ���� :";
textArray2[iIndex++]="NDS Context ���� :";
textArray2[iIndex++]="��ǰ״̬:";
//textArray2[iIndex++]="�Ͽ�";
//textArray2[iIndex++]="������";
textArray2[iIndex++]="NetWare Bindery ����";
textArray2[iIndex++]="ʹ�� Bindery ģʽ :";
//textArray2[iIndex++]="�ر�";
//textArray2[iIndex++]="����";
textArray2[iIndex++]="�ļ����������� :";
textArray2[iIndex++]="��ǰ״̬ :";
//textArray2[iIndex++]="�Ͽ�";
//textArray2[iIndex++]="������";
iIndex = 0;
//tcpip.htm
textArray3[iIndex++]="IP ����";
textArray3[iIndex++]="DHCP/BOOTP ״̬ :";
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
textArray4[iIndex++]="AppleTalk :";
textArray4[iIndex++]="��ӡ����Ϣ";
textArray4[iIndex++]="�˿����� :";
textArray4[iIndex++]="��ӡ������ :";
textArray4[iIndex++]="���ݸ�ʽ :";
iIndex = 0;
//SNMP.htm
textArray5[iIndex++]="SNMP ��������";
textArray5[iIndex++]="SNMP ���� 1 :";
//textArray5[iIndex++]="ֻ��";
//textArray5[iIndex++]="��д";
textArray5[iIndex++]="SNMP ���� 2 :";
//textArray5[iIndex++]="ֻ��";
//textArray5[iIndex++]="��д";
textArray5[iIndex++]="SNMP ��������";
textArray5[iIndex++]="���� SNMP ���� :";
//textArray5[iIndex++]="�ر�";
//textArray5[iIndex++]="����";
textArray5[iIndex++]="������֤ :";
//textArray5[iIndex++]="�ر�";
//textArray5[iIndex++]="����";
textArray5[iIndex++]="�����ַ 1 :";
textArray5[iIndex++]="�����ַ 2 :";
iIndex = 0;

//JOBLOG.htm
// Translate                                  only "Refresh " is to be translated
textArray6[iIndex++]='<input type=button value=" ˢ�� " onClick="window.location.reload()">';
textArray6[iIndex++]="��ӡ����";
textArray6[iIndex++]="����";
textArray6[iIndex++]="�û�";
textArray6[iIndex++]="����ִ��ʱ��";
textArray6[iIndex++]="Э��";
textArray6[iIndex++]="�˿�";
textArray6[iIndex++]="״̬";
textArray6[iIndex++]="��ӡ�ֽ���";
// Translate                                  only "Close " is to be translated
textArray6[iIndex++]='<input type=button value=" �ر� " onClick="window.close()">';
iIndex = 0;

//SMB.htm
textArray7[iIndex++]="������";
textArray7[iIndex++]="���������� :";
textArray7[iIndex++]="��������";
textArray7[iIndex++]="��ӡ�� :";


// out of Simplified Chinese
