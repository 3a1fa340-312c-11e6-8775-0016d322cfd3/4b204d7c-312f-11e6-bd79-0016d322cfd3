

//vaiable
menuindex = 0;
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Simplified Chinese
menuArray=['NP302','ϵͳ��Ϣ','ϵͳ����','��ӡ����','��������','ϵͳ����','�ۺ����'];
tabArray=['ϵͳ����','ϵͳ����','�ָ�ȱʡ','�û���������'];

//reset.htm 0
headArray[iIndex++] ="ϵͳ����";
//reset.htm
headArray[iIndex++] ="����������ӡ��������";
//upgrade.htm 0
headArray[iIndex++] ="ϵͳ����";
//upgrade.htm
headArray[iIndex++] ="������������ӡ�������İ汾 (��ѡ����ȷ�������ļ�, �����֪��Ӧ��ѡ����������ļ�, ����ϵ���صķ����̻���֧��)��";
//default.htm 0
headArray[iIndex++] ="�ָ�ȱʡ";
//default.htm
headArray[iIndex++] ="����ӡ�������ָ����������ã����в�������ʧ��";
//password.htm 0
headArray[iIndex++] ="�û���������";
//password.htm
headArray[iIndex++] ="������������޸����롣";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "ȷ������ӡ�������ָ�������������? (����! ��ǰ���в�������ʧ!)";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;ȷ��&nbsp;&nbsp;" class=input_more_back onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="�汾����";
textArray2[iIndex++]="ѡ�������ļ� :";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="������Ч" class=input_more_back onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>������ӡ��������</B></FONT>";
textArray3[iIndex++]="ȷ���������ò�������ӡ��������?";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;ȷ��&nbsp;&nbsp;" class=input_more_back onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

//password.htm
textArray3a[iIndex++]="�û���������";
textArray3a[iIndex++]="������:";
textArray3a[iIndex++]="ȷ��������:";
// Translate                               Only OK is to be translated
textArray3a[iIndex++]='<input type=button value="������Ч" class=input_more_back onClick="return CheckPwd(';
// Begin don't translate
textArray3a[iIndex++]="'RESTART.HTM');";
textArray3a[iIndex++]='">';
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

	//support.htm
textArray7[iIndex++] = "�ۺ����";
textArray7[iIndex++] = "Netcore�й���˾��2000��7����ʽ�����������������й�����������һ��Ӫ�����ģ�һ���������غ������з����ģ����ڹ�����Ҫ���ĳ��н�����10�����۰��´�������ȫ�������г���Netcore�й���˾ּ��Ϊ�����û��ṩ���á���Ч��ʵ�õ�ȫ����������������Ʒ��������������������������·���������ƽ�����ȶ��ϵ�С������������㵽�߲㽻����ȫ������ǰ�˿Ƽ����з���Ӧ�á�";
textArray7[iIndex++] = "��רע�û������ƶ�����Ӧ�á���Netcore�й���˾�������û�һ��ӭ����Ϣʱ������ս��";
textArray7[iIndex++] = "����ø����й��ڿ����缼������֧�֣����µ��ڿ��ⳤ;����֧�����ߣ�400-810-1616";
textArray7[iIndex++] = "�ڿƹٷ���վ";
textArray7[iIndex++] = "�ʺ�ȫ���û�����";
textArray7[iIndex++] = "�ڿƹٷ���վ(www.netcoretec.com)";
textArray7[iIndex++] = "�����������Ի�����µĲ�Ʒ��Ϣ����������汾���ͻ�������֧�������ṩ���꾡�ļ���֧�֣�����֧����Աһ��һ��Ϊ����������ǲ�Ʒʹ�ù��������������⡣";
textArray7[iIndex++] = "�ڿƱ�����վ";
textArray7[iIndex++] = "�ʺϱ�����ͨ�û�����";
textArray7[iIndex++] = "�ڿƱ�����վ(www.netcore.com.cn)";
textArray7[iIndex++] = "������վ�ʺϱ�����ͨ�û����ʣ���ʱ����������������汾��Ϊ����������ĸ������⡣";
iIndex = 0;
// out of Simplified Chinese
