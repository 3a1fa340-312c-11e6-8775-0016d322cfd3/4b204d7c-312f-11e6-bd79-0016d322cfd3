
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : English
tabArray=['PU201 ���t USB �L����A��','���A','�]�w','�䥦','���s�Ұ�','�t��','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>�����i�H���A�ק惡�L����A�����򥻳]�w�C<br>";
//ctcpip.htm
headArray[iIndex++] = "<BR>�����i�H���A�ק惡�L����A�� TCP/IP �������]�w���ءC";
//cnetware.htm
headArray[iIndex++] = "<BR>�����i�H���A�ק惡�L����A�� NetWare �������]�w���ءC";
//capple.htm 
headArray[iIndex++] ="<BR>�����i�H���A�ק惡�L����A�� AppleTalk �������]�w���ءC<br>";
//csnmp.htm
headArray[iIndex++] ="<BR>�����i�H���A�ק惡�L����A���� SNMP ���]�w�C";
//csmp.htm
headArray[iIndex++] ="<BR>�����i�H���A�ק惡�L����A�����L�n�����ھF�L������ɳ]�w�C";
iIndex = 0;

//csystem.htm
textArray0[iIndex++]="E-Mail ĵ�ܳ]�w";
textArray0[iIndex++]="E-Mail ĵ��:";
textArray0[iIndex++]="�~�H�l����A�� IP �a�}:";
textArray0[iIndex++]="�޲z�� E-mail �H�c:";
textArray0[iIndex++]="�t�γ]�w";
textArray0[iIndex++]="�˸m�W��:";
textArray0[iIndex++]="�s���H:";
textArray0[iIndex++]="�˸m��m:";
textArray0[iIndex++]="�t�κ޲z�̱K�X";
textArray0[iIndex++]="�K�X:";
textArray0[iIndex++]="�K�X�T�{:";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="�x�s�í��s�Ұ�" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
//CPRINTER.htm
textArray1[iIndex++]="����� - ���V�C�L�]�w";
textArray1[iIndex++]="�s����:";
textArray1[iIndex++]='<input type="button" value="�x�s�í��s�Ұ�" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
//CTCPIP.htm
textArray2[iIndex++]="TCP/IP �]�w";
textArray2[iIndex++]="�۰ʨ��o IP ��} (�ϥ� DHCP/BOOTP)";
textArray2[iIndex++]="���w IP ��}";
textArray2[iIndex++]="IP ��}:";
textArray2[iIndex++]="�l�����B�n:";
textArray2[iIndex++]="�w�]�h�D��:";
//crandvoo.htm
textArray2[iIndex++]="Rendezvous �]�w";
textArray2[iIndex++]="Rendezvous �]�w:";
//textArray2[iIndex++]="Disable";
//textArray2[iIndex++]="Enable";
textArray2[iIndex++]="�A�ȦW��:";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="�x�s�í��s�Ұ�" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="AppleTalk �]�w";
textArray3[iIndex++]="AppleTalk �ϰ�W��:";
textArray3[iIndex++]="�s����W��:";
textArray3[iIndex++]="�s����";
textArray3[iIndex++]="�L����Φ�:";
textArray3[iIndex++]="��Ʈ榡:";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="�x�s�í��s�Ұ�" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
//CSNMP.htm
textArray4[iIndex++]="SNMP �s��]�w";
textArray4[iIndex++]="�䴩 HP WebJetAdmin:";
textArray4[iIndex++]="�s��W�� 1:";
textArray4[iIndex++]="�s���v��:";
textArray4[iIndex++]="�s��W�� 2:";
textArray4[iIndex++]="�s���v��:";
textArray4[iIndex++]="SNMP �����]�w";
textArray4[iIndex++]="�ϥγ����ɧ�:";
textArray4[iIndex++]="�ǰe�T�{����:";
textArray4[iIndex++]="�����ؼ� IP ��} 1:";
textArray4[iIndex++]="�����ؼ� IP ��} 2:";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="�x�s�í��s�Ұ�" onClick="return SaveSetting(';
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
textArray5[iIndex++]='<INPUT TYPE=button VALUE=" ���� " onClick="window.close()">';
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
textArray7[iIndex++]="�^��W�@��";
iIndex = 0;

// CNETWARE.htm
textArray8[iIndex++]="�򥻳]�w";
textArray8[iIndex++]="�L����A���W��:";
textArray8[iIndex++]="���߮ɶ�:";
textArray8[iIndex++]="&nbsp;�� (�̤p: 3 ��, �̤j: 29 ��)";
textArray8[iIndex++]="�n�J NetWare ���K�X:";
textArray8[iIndex++]="NetWare NDS �]�w";
textArray8[iIndex++]="�ϥ� NDS �Ҧ�:";
textArray8[iIndex++]="NDS Tree �W��:";
textArray8[iIndex++]="NDS Context �W��:";
textArray8[iIndex++]="NetWare Bindery �]�w";
textArray8[iIndex++]="�ϥ� Bindery �Ҧ�:";
textArray8[iIndex++]="�ɮצ��A���W��:";
textArray8[iIndex++]='<input type="button" value="�x�s�í��s�Ұ�" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.htm
textArray9[iIndex++]="�u�@�s��";
textArray9[iIndex++]="�W��:";
textArray9[iIndex++]="�L����@�ΦW��";
textArray9[iIndex++]="�@�ΦW��:";
textArray9[iIndex++]='<input type="button" value="�x�s�í��s�Ұ�" onClick="return CheckSMB(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;
// out of Chinese
