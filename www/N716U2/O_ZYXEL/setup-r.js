
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Russian
tabArray=['���������','������������','�������������','������������','�������','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>��� �������� ��������� ��������� ����� ��������� ��������� ������� ������.<br>";
//ctcpip.htm
headArray[iIndex++] = "<BR>��� �������� ��������� ��������� ��������� TCP/IP ������� ������.";
//cnetware.htm
headArray[iIndex++] = "<BR>��� �������� ��������� ��������� ��������� NetWare ������� ������.";
//capple.htm 
headArray[iIndex++] ="<BR>��� �������� ��������� ��������� ��������� AppleTalk ������� ������.<br>";
//csnmp.htm
headArray[iIndex++] ="<BR>��� �������� ��������� ��������� ��������� SNMP ������� ������.";
//csmp.htm
headArray[iIndex++] ="<BR>�� ���� �������� ������������ ������� ��������� ����������� ������� � ��������� ��� ����� Microsoft Windows.";
iIndex = 0;

// CSYSTEM.HTM
textArray0[iIndex++]="��������� ���������";
textArray0[iIndex++]="��� ������� ������:";
textArray0[iIndex++]="������������� ����:";
textArray0[iIndex++]="�������������� �������:";
textArray0[iIndex++]="������ ��������������";
textArray0[iIndex++]="������:";
textArray0[iIndex++]="��������� ������:";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="��������� � ������������� �������" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
// CPRINTER.HTM
textArray1[iIndex++]="������������ ���� - ��������� ������������� ������";
textArray1[iIndex++]="���� �������� 1:";
textArray1[iIndex++]='<input type="button" value="��������� � ������������� �������" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
// CTCPIP.HTM
textArray2[iIndex++]="��������� TCP/IP";
textArray2[iIndex++]="��������� ��������� TCP/IP ������������� (�� ������� DHCP/BOOTP)";
textArray2[iIndex++]="������������ ��������� ��������� TCP/IP:";
textArray2[iIndex++]="����� IP:";
textArray2[iIndex++]="����� �������:";
textArray2[iIndex++]="������������� �� ���������:";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="��������� � ������������� �������" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="��������� AppleTalk";
textArray3[iIndex++]="���� AppleTalk:";
textArray3[iIndex++]="��� �����:";
textArray3[iIndex++]="������������ ��������";
textArray3[iIndex++]="���:";
textArray3[iIndex++]="�������� ��������:";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="��������� � ������������� �������" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
// CSNMP.HTM
textArray4[iIndex++]="��������� ��������� SNMP:";
textArray4[iIndex++]="��������� ������ HP WebJetAdmin:";
textArray4[iIndex++]="��� SNMP-���������� 1:";
textArray4[iIndex++]="������� �������:";
textArray4[iIndex++]="��� SNMP-���������� 2:";
textArray4[iIndex++]="������� �������:";
textArray4[iIndex++]="��������� SNMP-�������";
textArray4[iIndex++]="�������� SNMP-�������:";
textArray4[iIndex++]="������ ������ ��������������:";
textArray4[iIndex++]="����� SNMP-��������� 1:";
textArray4[iIndex++]="����� SNMP-��������� 2:";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="��������� � ������������� �������" onClick="return SaveSetting(';
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
textArray7[iIndex++]="������";
textArray7[iIndex++]="���������";
iIndex = 0;

// CNETWARE.HTM
textArray8[iIndex++]="����� ���������";
textArray8[iIndex++]="��� ������� ������:";
textArray8[iIndex++]="����� ������:";
textArray8[iIndex++]="&nbsp;����� � �������� (���: 3 ���, ����: 29 ���)";
textArray8[iIndex++]="������ �������:";
textArray8[iIndex++]="��������� NetWare NDS";
textArray8[iIndex++]="������������ ����� NDS:";
textArray8[iIndex++]="��� ������ NDS:";
textArray8[iIndex++]="��� ��������� NDS:";
textArray8[iIndex++]="��������� ���� �������� NetWare Bindery";
textArray8[iIndex++]="����� ���� �������� Bindery:";
textArray8[iIndex++]="�������� ������:";
textArray8[iIndex++]='<input type="button" value="��������� � ������������� �������" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.HTM
textArray9[iIndex++]="������� ������";
textArray9[iIndex++]="��������:";
textArray9[iIndex++]="����� ��������";
textArray9[iIndex++]="�������:";
textArray9[iIndex++]='<input type="button" value="��������� � ������������� �������" onClick="return SaveSetting(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;
// out of Russian
