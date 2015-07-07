
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Chinese
tabArray=['狀態','設定','其他','重新啟動','系統','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>您可以使用此設定頁配置列印伺服器的一般系統設定。<br>";
//ctcpip.htm
headArray[iIndex++] = "<BR>您可以使用此設定頁配置列印伺服器的 TCP/IP 設定。";
//cnetware.htm
headArray[iIndex++] = "<BR>您可以使用此設定頁配置列印伺服器的 NetWare 功能。";
//capple.htm 
headArray[iIndex++] ="<BR>您可以使用此設定頁配置列印伺服器的 AppleTalk 設定。<br>";
//csnmp.htm
headArray[iIndex++] ="<BR>您可以使用此設定頁配置列印伺服器的 SNMP 設定。";
//csmp.htm
headArray[iIndex++] ="<BR>此頁顯示 Microsoft Windows 網路的印表機共用設定。";
iIndex = 0;

// CSYSTEM.HTM
textArray0[iIndex++]="系統設定";
textArray0[iIndex++]="列印伺服器名稱：";
textArray0[iIndex++]="系統聯絡人：";
textArray0[iIndex++]="系統位置：";
textArray0[iIndex++]="管理員密碼";
textArray0[iIndex++]="密碼：";
textArray0[iIndex++]="重新輸入密碼：";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="儲存與重新啟動" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
// CPRINTER.HTM
textArray1[iIndex++]="並列埠－雙向設定";
textArray1[iIndex++]="印表機埠 1：";
textArray1[iIndex++]='<input type="button" value="儲存與重新啟動" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
// CTCPIP.HTM
textArray2[iIndex++]="TCP/IP 設定";
textArray2[iIndex++]="自動取得 TCP/IP 設定 (使用 DHCP/BOOTP)";
textArray2[iIndex++]="使用下列 TCP/IP 設定";
textArray2[iIndex++]="IP 位址：";
textArray2[iIndex++]="子網路遮罩：";
textArray2[iIndex++]="預設路由器：";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="儲存與重新啟動" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="AppleTalk 設定";
textArray3[iIndex++]="AppleTalk 區域：";
textArray3[iIndex++]="連接埠名稱：";
textArray3[iIndex++]="印表機設定";
textArray3[iIndex++]="類型：";
textArray3[iIndex++]="二進位通訊協定：";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="儲存與重新啟動" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
// CSNMP.HTM
textArray4[iIndex++]="SNMP 社群設定";
textArray4[iIndex++]="支援 HP WebJetAdmin：";
textArray4[iIndex++]="SNMP 社群名稱 1：";
textArray4[iIndex++]="權限：";
textArray4[iIndex++]="SNMP 社群名稱 2：";
textArray4[iIndex++]="權限：";
textArray4[iIndex++]="SNMP 設陷設定";
textArray4[iIndex++]="傳送 SNMP 設陷：";
textArray4[iIndex++]="使用認證設陷：";
textArray4[iIndex++]="設陷位址 1：";
textArray4[iIndex++]="設陷位址 2：";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="儲存與重新啟動" onClick="return SaveSetting(';
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
textArray7[iIndex++]="錯誤";
textArray7[iIndex++]="返回";
iIndex = 0;

// CNETWARE.HTM
textArray8[iIndex++]="一般設定";
textArray8[iIndex++]="列印伺服器名稱：";
textArray8[iIndex++]="輪詢時間：";
textArray8[iIndex++]="&nbsp;秒數 (最少：3 秒，最多：29 秒)";
textArray8[iIndex++]="登入密碼：";
textArray8[iIndex++]="NetWare NDS 設定";
textArray8[iIndex++]="使用 NDS 模式：";
textArray8[iIndex++]="NDS 樹狀目錄名稱：";
textArray8[iIndex++]="NDS 內容名稱：";
textArray8[iIndex++]="NetWare Bindery 設定";
textArray8[iIndex++]="使用 Bindery 模式：";
textArray8[iIndex++]="檔案伺服器名稱：";
textArray8[iIndex++]='<input type="button" value="儲存與重新啟動" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.HTM
textArray9[iIndex++]="工作群組";
textArray9[iIndex++]="名稱：";
textArray9[iIndex++]="共用名稱";
textArray9[iIndex++]="印表機：";
textArray9[iIndex++]='<input type="button" value="儲存與重新啟動" onClick="return SaveSetting(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;
// out of Chinese
