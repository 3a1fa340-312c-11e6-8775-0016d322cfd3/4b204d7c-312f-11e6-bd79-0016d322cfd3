

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Traditional Chinese
//tabArray=['<IMG SRC=images/LOGO.gif>','Status','Setup','Misc','Restart','Factory Default','Firmware Upgrade','Restart'];
tabArray=['�L�u/���u�������L����A��','���A','�]�w','�䥦','���s�Ұ�','�^�_�X�t��','����ɯ�','���s�Ұ�'];

	//upgrade.htm
headArray[iIndex++] ="<br>�����i�H���z�ɯŦL����A��������C<BR><font color=red>����:</font> �b����ɯŤ��e, �нT�w�z������O���T���C���p�z�����D�ӥέ��ض���, �лP�t���p���H�M�D�޳N�W���䴩�C";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "�I���u<b>�T�w</b>�v �H�^�_�X�t���w�]�ȡC<BR><FONT CLASS=F1 COLOR=#FF3300><B>�Ъ`�N! �Ҧ��ثe���]�w�ȳ��N�Q�ٰ��C</B></FONT>";
textArray1[iIndex++] = "�^�_�X�t��, ���]�A TCP/IP �]�w. (����)";
textArray1[iIndex++] = "�b�����s�Ұʫ�, �z���M��Υثe�� IP �s�쥦�C";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;�T�w&nbsp;&nbsp;" onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="����ɯ�";
textArray2[iIndex++]="����ɮ�:";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="����ɯ�" onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="�����i�H���z���s�ҰʦL����A���C<br>";
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>���s�ҰʦL����A��</B></FONT><br><br>�A�Q�n���s�ҰʦL����A����?<br><br>";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;�T�w&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

	//restart.htm
textArray4[iIndex++] = "���s�Ұʤ� ...";
textArray4[iIndex++] = "�е��ݦL����A�����s�ҰʡC";
iIndex = 0;
	//urestart.htm
textArray5[iIndex++] = "�ɯŦ��\ !";
textArray5[iIndex++] = "����ɯŦ��\��, �L����A���N�|�۰ʭ��s�ҰʡC�е��ݦL����A�����s�ҰʡC";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "�^�_���X�t���]�w�Ȥ� ...";
textArray6[iIndex++] = "�^�_���X�t���]�w�ȫ�, �L����A���N�|�۰ʭ��s�ҰʡC<BR><BR>�е��ݦL����A�����s�ҰʡC";
iIndex = 0;

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=big5">');
}

// out of Traditional Chinese
