//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

tabArray=['<IMG SRC=images/LOGO.gif>','Status','Setup','Misc','Restart','System','Printer','Wireless','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
tabArray=['無線/有線雙介面印表伺服器','狀態','設定','其它','重新啟動','系統','印表機','無線','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
//Language : Traditional Chinese

//system.htm
headArray[iIndex++] = "<BR>本頁可以顯示有關此印表伺服器的狀態與資訊。<BR>";
//printer.htm
headArray[iIndex++] = "<BR>本頁可以顯示與印表伺服器連接的印表機的狀態與資訊。<BR>附註: 假如您的印表機不支援雙向列印功能, 印表機的廠牌, 型號及列印語言將不會被顯示出來。";
//wlan.htm
headArray[iIndex++] = "<BR>本頁顯示目前此印表伺服器的無線設定值和無線狀態。";
//tcpip.htm
headArray[iIndex++] = "<BR>本頁顯示目前此印表伺服器的 TCP/IP 設定值。<BR>";
//netware.htm
headArray[iIndex++] = "<BR>本頁顯示目前此印表伺服器的 NetWare 設定值。<BR>";
//apple.htm
headArray[iIndex++] = "<BR>本頁顯示目前此印表伺服器的 AppleTalk 設定值。<BR>";
//snmp.htm
headArray[iIndex++] = "<BR>本頁顯示目前此印表伺服器的 SNMP 設定值。<BR>";
//Smb.htm
headArray[iIndex++] = "<BR>本頁顯示目前此印表伺服器在微軟網路芳鄰中的印表機分享設定。<BR>";
iIndex = 0;



//system.htm
textArray0[iIndex++]="系統資訊";
textArray0[iIndex++]="裝置名稱 :";
textArray0[iIndex++]="聯絡人 :";
textArray0[iIndex++]="裝置位置 :";
textArray0[iIndex++]="開機時間 :";
textArray0[iIndex++]="韌體版本 :";
textArray0[iIndex++]="網路卡位址 :";
textArray0[iIndex++]="E-mail 警示 :";
textArray0[iIndex++]="停用";
textArray0[iIndex++]="啟用";
//PRINTJOB.htm
textArray0[iIndex++]="目前的列印工作";
textArray0[iIndex++]="工作編號";
textArray0[iIndex++]="使用者";
textArray0[iIndex++]="花費時間";
textArray0[iIndex++]="通訊協定";
textArray0[iIndex++]="列印埠";
textArray0[iIndex++]="狀態";
textArray0[iIndex++]="位元組數";
textArray0[iIndex++]="觀看列印工作紀錄";
iIndex = 0;

//Printer.htm
textArray1[iIndex++]="印表機";
textArray1[iIndex++]="印表機廠牌 :";
textArray1[iIndex++]="印表機型號 :";
textArray1[iIndex++]="支援的列印語言 :";
textArray1[iIndex++]="目前狀態 :";
textArray1[iIndex++]="待機中";
textArray1[iIndex++]="缺紙";
textArray1[iIndex++]="未連接或印表機離線";
textArray1[iIndex++]="列印中";
iIndex = 0;

//NETWARE.htm
textArray2[iIndex++]="基本設定";
textArray2[iIndex++]="印表伺服器名稱 :";
textArray2[iIndex++]="輪詢時間 :";
textArray2[iIndex++]="秒";
textArray2[iIndex++]="NetWare NDS 設定";
textArray2[iIndex++]="使用 NDS 模式 :";
textArray2[iIndex++]="停用";
textArray2[iIndex++]="啟用";
textArray2[iIndex++]="NDS Tree 名稱 :";
textArray2[iIndex++]="NDS Context 名稱 :";
textArray2[iIndex++]="目前狀態 :";
textArray2[iIndex++]="未連接";
textArray2[iIndex++]="已連接";
textArray2[iIndex++]="NetWare Bindery 設定";
textArray2[iIndex++]="使用 Bindery 模式 :";
textArray2[iIndex++]="停用";
textArray2[iIndex++]="啟用";
textArray2[iIndex++]="檔案伺服器名稱 :";
textArray2[iIndex++]="目前狀態 :";
textArray2[iIndex++]="未連接";
textArray2[iIndex++]="已連接";
iIndex = 0;
//tcpip.htm
textArray3[iIndex++]="TCP/IP 設定";
textArray3[iIndex++]="使用 DHCP/BOOTP :";
textArray3[iIndex++]="IP 位址 :";
textArray3[iIndex++]="子網路遮罩 :";
textArray3[iIndex++]="閘道器 :";
//randvoo.htm
textArray3[iIndex++]="Rendezvous 設定";
textArray3[iIndex++]="Rendezvous 設定 :";
//textArray3[iIndex++]="停用";
//textArray3[iIndex++]="啟用";
textArray3[iIndex++]="服務名稱 :";
iIndex = 0;
//APPLE.htm
textArray4[iIndex++]="AppleTalk 設定";
textArray4[iIndex++]="AppleTalk 區域名稱 :";
textArray4[iIndex++]="連接埠";
textArray4[iIndex++]="連接埠名稱 :";
textArray4[iIndex++]="印表機形式 :";
textArray4[iIndex++]="資料格式 :";
iIndex = 0;
//SNMP.htm
textArray5[iIndex++]="SNMP 群體設定";
textArray5[iIndex++]="群體 1 :";
textArray5[iIndex++]="只能讀";
textArray5[iIndex++]="讀寫皆可";
textArray5[iIndex++]="群體 2 :";
textArray5[iIndex++]="只能讀";
textArray5[iIndex++]="讀寫皆可";
textArray5[iIndex++]="SNMP 陷阱設定";
textArray5[iIndex++]="使用陷阱補抓 :";
textArray5[iIndex++]="停用";
textArray5[iIndex++]="啟用";
textArray5[iIndex++]="傳送確認陷阱 :";
textArray5[iIndex++]="停用";
textArray5[iIndex++]="啟用";
textArray5[iIndex++]="陷阱目標 IP 位址 1 :";
textArray5[iIndex++]="陷阱目標 IP 位址 2 :";
iIndex = 0;

//JOBLOG.htm
// Translate                                  only "Refresh " is to be translated
textArray6[iIndex++]='<input type=button value=" 更新 " onClick="window.location.reload()">';
textArray6[iIndex++]="列印工作紀錄";
textArray6[iIndex++]="工作編號";
textArray6[iIndex++]="使用者";
textArray6[iIndex++]="花費時間";
textArray6[iIndex++]="通訊協定";
textArray6[iIndex++]="列印埠";
textArray6[iIndex++]="狀態";
textArray6[iIndex++]="位元組數";
// Translate                                  only "Close " is to be translated
textArray6[iIndex++]='<input type=button value=" 關閉 " onClick="window.close()">';
iIndex = 0;

//SMB.htm
textArray7[iIndex++]="工作群組";
textArray7[iIndex++]="名稱 :";
textArray7[iIndex++]="印表機共用名稱";
textArray7[iIndex++]="連接埠 :";
iIndex = 0;

//WLAN.htm
textArray8[iIndex++]="<BR><BR><FONT COLOR=RED>注意 ! 印表伺服器目前為診斷模式。</FONT><BR>";
textArray8[iIndex++]="無線資訊";
textArray8[iIndex++]="無線模式:";
textArray8[iIndex++]="用戶端模式";
textArray8[iIndex++]="無線基地台 AP 模式";
textArray8[iIndex++]="網路類型:";
textArray8[iIndex++]="Infrastructure";
textArray8[iIndex++]="Ad-Hoc";
textArray8[iIndex++]="無線基地台的 MAC 地址:";
textArray8[iIndex++]="SSID:";
textArray8[iIndex++]="頻道:";
textArray8[iIndex++]="傳輸模式:";
textArray8[iIndex++]="只使用 B";
textArray8[iIndex++]="只使用 G";
textArray8[iIndex++]="自動選擇 B/G/N";
textArray8[iIndex++]="自動選擇 B/G";
textArray8[iIndex++]="傳輸速率:";
textArray8[iIndex++]="Rx 信號強度:";
textArray8[iIndex++]="連線品質:";
textArray8[iIndex++]="網路驗證:";
textArray8[iIndex++]="共享金鑰";
textArray8[iIndex++]="WPA-PSK";
textArray8[iIndex++]="WPA2-PSK";
textArray8[iIndex++]="開放式系統";
textArray8[iIndex++]="金鑰加密:";
textArray8[iIndex++]="WEP 64 位元";
textArray8[iIndex++]="WEP 128 位元";
textArray8[iIndex++]="無";
textArray8[iIndex++]="停用";
iIndex = 0;

// Character Encoding
function CharacterEncoding()
{
  document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=big5">');
  
}

// out of Traditional Chinese

