
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Chinese
tabArray=['���A','�]�w','��L','���s�Ұ�','�t��','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>�z�i�H�ϥΦ��]�w���t�m�C�L���A�����@��t�γ]�w�C<br>";
//ctcpip.htm
headArray[iIndex++] = "<BR>�z�i�H�ϥΦ��]�w���t�m�C�L���A���� TCP/IP �]�w�C";
//cnetware.htm
headArray[iIndex++] = "<BR>�z�i�H�ϥΦ��]�w���t�m�C�L���A���� NetWare �\��C";
//capple.htm 
headArray[iIndex++] ="<BR>�z�i�H�ϥΦ��]�w���t�m�C�L���A���� AppleTalk �]�w�C<br>";
//csnmp.htm
headArray[iIndex++] ="<BR>�z�i�H�ϥΦ��]�w���t�m�C�L���A���� SNMP �]�w�C";
//csmp.htm
headArray[iIndex++] ="<BR>������� Microsoft Windows �������L����@�γ]�w�C";
iIndex = 0;

// CSYSTEM.HTM
textArray0[iIndex++]="�t�γ]�w";
textArray0[iIndex++]="�C�L���A���W�١G";
textArray0[iIndex++]="�t���p���H�G";
textArray0[iIndex++]="�t�Φ�m�G";
textArray0[iIndex++]="�޲z���K�X";
textArray0[iIndex++]="�K�X�G";
textArray0[iIndex++]="���s��J�K�X�G";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="�x�s�P���s�Ұ�" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
// CPRINTER.HTM
textArray1[iIndex++]="�æC������V�]�w";
textArray1[iIndex++]="�L����� 1�G";
textArray1[iIndex++]='<input type="button" value="�x�s�P���s�Ұ�" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
// CTCPIP.HTM
textArray2[iIndex++]="TCP/IP �]�w";
textArray2[iIndex++]="�۰ʨ��o TCP/IP �]�w (�ϥ� DHCP/BOOTP)";
textArray2[iIndex++]="�ϥΤU�C TCP/IP �]�w";
textArray2[iIndex++]="IP ��}�G";
textArray2[iIndex++]="�l�����B�n�G";
textArray2[iIndex++]="�w�]���Ѿ��G";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="�x�s�P���s�Ұ�" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="AppleTalk �]�w";
textArray3[iIndex++]="AppleTalk �ϰ�G";
textArray3[iIndex++]="�s����W�١G";
textArray3[iIndex++]="�L����]�w";
textArray3[iIndex++]="�����G";
textArray3[iIndex++]="�G�i��q�T��w�G";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="�x�s�P���s�Ұ�" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
// CSNMP.HTM
textArray4[iIndex++]="SNMP ���s�]�w";
textArray4[iIndex++]="�䴩 HP WebJetAdmin�G";
textArray4[iIndex++]="SNMP ���s�W�� 1�G";
textArray4[iIndex++]="�v���G";
textArray4[iIndex++]="SNMP ���s�W�� 2�G";
textArray4[iIndex++]="�v���G";
textArray4[iIndex++]="SNMP �]���]�w";
textArray4[iIndex++]="�ǰe SNMP �]���G";
textArray4[iIndex++]="�ϥλ{�ҳ]���G";
textArray4[iIndex++]="�]����} 1�G";
textArray4[iIndex++]="�]����} 2�G";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="�x�s�P���s�Ұ�" onClick="return SaveSetting(';
// Begin don't translate
textArray4[iIndex++]="'RESTART.HTM');";
textArray4[iIndex++]='">';
iIndex = 0;
// End don't translate
//keyhelp.htm
//textArray5[iIndex++]="<b>WEP Key Format</b>";
//textArray5[iIndex++]="An alphanumeric character is 'a' through 'z', 'A' through 'Z', and '0' through '9'.";
//textArray5[iIndex++]="A hexadecimal digit is '0' through '9' and 'A' through 'F'.";
//textArray5[iIndex++]="Depending on the key format you select:";
//textArray5[iIndex++]="For 64-bit (sometimes called 40-bit) WEP encryption, enter the key which contains 5 alphanumeric characters or 10 hexadecimal digits. For example: AbZ12 (alphanumeric format) or ABCDEF1234 (hexadecimal format).";
//textArray5[iIndex++]="For 128-bit WEP encryption, enter the key which contains 13 alphanumeric characters or 26 hexadecimal digits.";
// Translate                                  only "Close" is to be translated
textArray5[iIndex++]='<INPUT TYPE=button VALUE=" Close " onClick="window.close()">';
iIndex = 0;
//browser.htm
//textArray6[iIndex++]="SSID";
//textArray6[iIndex++]="AP's MAC Address or BSSID";
//textArray6[iIndex++]="Channel";
//textArray6[iIndex++]="Type";
//textArray6[iIndex++]="WEP/WPA-PSK";
//textArray6[iIndex++]="Signal Strength";
// Translate                                  only "Rescan" is to be translated
//textArray6[iIndex++]='<INPUT TYPE=submit VALUE="Rescan">';
// Translate                                  only "Close" is to be translated
//textArray6[iIndex++]='<INPUT TYPE=button VALUE=" Close " onClick="window.close()">';
//iIndex = 0;
// End don't translate
// ERROR.htm
textArray7[iIndex++]="���~";
textArray7[iIndex++]="��^";
iIndex = 0;

// CNETWARE.HTM
textArray8[iIndex++]="�@��]�w";
textArray8[iIndex++]="�C�L���A���W�١G";
textArray8[iIndex++]="���߮ɶ��G";
textArray8[iIndex++]="&nbsp;��� (�̤֡G3 ��A�̦h�G29 ��)";
textArray8[iIndex++]="�n�J�K�X�G";
textArray8[iIndex++]="NetWare NDS �]�w";
textArray8[iIndex++]="�ϥ� NDS �Ҧ��G";
textArray8[iIndex++]="NDS �𪬥ؿ��W�١G";
textArray8[iIndex++]="NDS ���e�W�١G";
textArray8[iIndex++]="NetWare Bindery �]�w";
textArray8[iIndex++]="�ϥ� Bindery �Ҧ��G";
textArray8[iIndex++]="�ɮצ��A���W�١G";
textArray8[iIndex++]='<input type="button" value="�x�s�P���s�Ұ�" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.HTM
textArray9[iIndex++]="�u�@�s��";
textArray9[iIndex++]="�W�١G";
textArray9[iIndex++]="�@�ΦW��";
textArray9[iIndex++]="�L����G";
textArray9[iIndex++]='<input type="button" value="�x�s�P���s�Ұ�" onClick="return SaveSetting(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;
// out of Chinese
