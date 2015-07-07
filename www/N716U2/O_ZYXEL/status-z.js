//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

tabArray=['狀態','設定','其他','重新啟動','系統','印表機','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
//Language : Chinese

//system.htm
headArray[iIndex++] = "<BR>此頁顯示列印伺服器的一般系統資訊。<BR>";
//printer.htm
headArray[iIndex++] = "<BR>此頁顯示目前與列印伺服器連接的印表機資訊。<BR>注意：如果印表機不支援雙向功能，某些資訊可能無法正確顯示。";
//tcpip.htm
headArray[iIndex++] = "<BR>此頁顯示列印伺服器目前的 TCP/IP 設定。<BR>";
//netware.htm
headArray[iIndex++] = "<BR>此頁顯示列印伺服器目前的 NetWare 設定。<BR>";
//apple.htm
headArray[iIndex++] = "<BR>此頁顯示列印伺服器目前的 AppleTalk 設定。<BR>";
//snmp.htm
headArray[iIndex++] = "<BR>此頁顯示列印伺服器目前的 SNMP 設定。<BR>";
//Smb.htm
headArray[iIndex++] = "<BR>此頁顯示 Microsoft Windows 網路的印表機共用設定<BR>";
iIndex = 0;

// SYSTEM.HTM
textArray0[iIndex++]="系統資訊";
textArray0[iIndex++]="列印伺服器名稱：";
textArray0[iIndex++]="系統聯絡人：";
textArray0[iIndex++]="系統位置：";
textArray0[iIndex++]="系統存留時間：";
textArray0[iIndex++]="韌體版本：";
textArray0[iIndex++]="MAC 位址：";
iIndex = 0;

// PRINTER.HTM
textArray1[iIndex++]="印表機資訊";
textArray1[iIndex++]="製造商";
textArray1[iIndex++]="型號";
textArray1[iIndex++]="支援列印語言";
textArray1[iIndex++]="目前狀態";
iIndex = 0;

// NETWARE.HTM
textArray2[iIndex++]="一般設定";
textArray2[iIndex++]="列印伺服器名稱：";
textArray2[iIndex++]="輪詢時間：";
textArray2[iIndex++]="NetWare NDS 設定";
textArray2[iIndex++]="使用 NDS 模式：";
textArray2[iIndex++]="NDS 樹狀目錄名稱：";
textArray2[iIndex++]="NDS 內容名稱：";
textArray2[iIndex++]="目前狀態：";
textArray2[iIndex++]="NetWare Bindery 設定";
textArray2[iIndex++]="使用 Bindery 模式：";
textArray2[iIndex++]="檔案伺服器名稱：";
textArray2[iIndex++]="目前狀態：";
iIndex = 0;

// TCPIP.HTM
textArray3[iIndex++]="TCP/IP 設定";
textArray3[iIndex++]="使用 DHCP/BOOTP：";
textArray3[iIndex++]="IP 位址：";
textArray3[iIndex++]="子網路遮罩：";
textArray3[iIndex++]="閘道：";
iIndex = 0;

// APPLE.HTM
textArray4[iIndex++]="AppleTalk 設定";
textArray4[iIndex++]="AppleTalk:";
textArray4[iIndex++]="印表機資訊";
textArray4[iIndex++]="連接埠名稱：";
textArray4[iIndex++]="印表機類型：";
textArray4[iIndex++]="資料格式：";
iIndex = 0;

// SNMP.HTM
textArray5[iIndex++]="SNMP 社群設定";
textArray5[iIndex++]="SNMP 社群 1：";
textArray5[iIndex++]="SNMP 社群 2：";
textArray5[iIndex++]="SNMP 設陷設定";
textArray5[iIndex++]="傳送 SNMP 設陷：";
textArray5[iIndex++]="使用認證設陷：";
textArray5[iIndex++]="設陷位址 1：";
textArray5[iIndex++]="設陷位址 2：";
iIndex = 0;



// SMB.HTM
textArray7[iIndex++]="工作群組";
textArray7[iIndex++]="名稱：";
textArray7[iIndex++]="共用名稱";
textArray7[iIndex++]="印表機：";


// out of Chinese
