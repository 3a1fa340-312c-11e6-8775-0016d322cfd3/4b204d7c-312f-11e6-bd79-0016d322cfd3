
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : English
tabArray=['PU201 高速 USB 印表伺服器','狀態','設定','其它','重新啟動','系統','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>本頁可以讓你修改此印表伺服器的基本設定。<br>";
//ctcpip.htm
headArray[iIndex++] = "<BR>本頁可以讓你修改此印表伺服器 TCP/IP 相關的設定項目。";
//cnetware.htm
headArray[iIndex++] = "<BR>本頁可以讓你修改此印表伺服器 NetWare 相關的設定項目。";
//capple.htm 
headArray[iIndex++] ="<BR>本頁可以讓你修改此印表伺服器 AppleTalk 相關的設定項目。<br>";
//csnmp.htm
headArray[iIndex++] ="<BR>本頁可以讓你修改此印表伺服器的 SNMP 的設定。";
//csmp.htm
headArray[iIndex++] ="<BR>本頁可以讓你修改此印表伺服器的微軟網路芳鄰印表機分享設定。";
iIndex = 0;

//csystem.htm
textArray0[iIndex++]="E-Mail 警示設定";
textArray0[iIndex++]="E-Mail 警示:";
textArray0[iIndex++]="外寄郵件伺服器 IP 地址:";
textArray0[iIndex++]="管理者 E-mail 信箱:";
textArray0[iIndex++]="系統設定";
textArray0[iIndex++]="裝置名稱:";
textArray0[iIndex++]="連絡人:";
textArray0[iIndex++]="裝置位置:";
textArray0[iIndex++]="系統管理者密碼";
textArray0[iIndex++]="密碼:";
textArray0[iIndex++]="密碼確認:";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="儲存並重新啟動" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
//CPRINTER.htm
textArray1[iIndex++]="平行埠 - 雙向列印設定";
textArray1[iIndex++]="連接埠:";
textArray1[iIndex++]='<input type="button" value="儲存並重新啟動" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
//CTCPIP.htm
textArray2[iIndex++]="TCP/IP 設定";
textArray2[iIndex++]="自動取得 IP 位址 (使用 DHCP/BOOTP)";
textArray2[iIndex++]="指定 IP 位址";
textArray2[iIndex++]="IP 位址:";
textArray2[iIndex++]="子網路遮罩:";
textArray2[iIndex++]="預設閘道器:";
//crandvoo.htm
textArray2[iIndex++]="Rendezvous 設定";
textArray2[iIndex++]="Rendezvous 設定:";
//textArray2[iIndex++]="Disable";
//textArray2[iIndex++]="Enable";
textArray2[iIndex++]="服務名稱:";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="儲存並重新啟動" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="AppleTalk 設定";
textArray3[iIndex++]="AppleTalk 區域名稱:";
textArray3[iIndex++]="連接埠名稱:";
textArray3[iIndex++]="連接埠";
textArray3[iIndex++]="印表機形式:";
textArray3[iIndex++]="資料格式:";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="儲存並重新啟動" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
//CSNMP.htm
textArray4[iIndex++]="SNMP 群體設定";
textArray4[iIndex++]="支援 HP WebJetAdmin:";
textArray4[iIndex++]="群體名稱 1:";
textArray4[iIndex++]="群體權限:";
textArray4[iIndex++]="群體名稱 2:";
textArray4[iIndex++]="群體權限:";
textArray4[iIndex++]="SNMP 陷阱設定";
textArray4[iIndex++]="使用陷阱補抓:";
textArray4[iIndex++]="傳送確認陷阱:";
textArray4[iIndex++]="陷阱目標 IP 位址 1:";
textArray4[iIndex++]="陷阱目標 IP 位址 2:";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="儲存並重新啟動" onClick="return SaveSetting(';
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
textArray5[iIndex++]='<INPUT TYPE=button VALUE=" 關閉 " onClick="window.close()">';
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
textArray7[iIndex++]="回到上一頁";
iIndex = 0;

// CNETWARE.htm
textArray8[iIndex++]="基本設定";
textArray8[iIndex++]="印表伺服器名稱:";
textArray8[iIndex++]="輪詢時間:";
textArray8[iIndex++]="&nbsp;秒 (最小: 3 秒, 最大: 29 秒)";
textArray8[iIndex++]="登入 NetWare 的密碼:";
textArray8[iIndex++]="NetWare NDS 設定";
textArray8[iIndex++]="使用 NDS 模式:";
textArray8[iIndex++]="NDS Tree 名稱:";
textArray8[iIndex++]="NDS Context 名稱:";
textArray8[iIndex++]="NetWare Bindery 設定";
textArray8[iIndex++]="使用 Bindery 模式:";
textArray8[iIndex++]="檔案伺服器名稱:";
textArray8[iIndex++]='<input type="button" value="儲存並重新啟動" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.htm
textArray9[iIndex++]="工作群組";
textArray9[iIndex++]="名稱:";
textArray9[iIndex++]="印表機共用名稱";
textArray9[iIndex++]="共用名稱:";
textArray9[iIndex++]='<input type="button" value="儲存並重新啟動" onClick="return CheckSMB(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;
// out of Chinese
