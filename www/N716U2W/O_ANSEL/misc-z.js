

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Traditional Chinese
//tabArray=['<IMG SRC=images/LOGO.gif>','Status','Setup','Misc','Restart','Factory Default','Firmware Upgrade','Restart'];
tabArray=['無線/有線雙介面印表伺服器','狀態','設定','其它','重新啟動','回復出廠值','韌體升級','重新啟動'];

	//upgrade.htm
headArray[iIndex++] ="<br>本頁可以讓您升級印表伺服器的韌體。<BR><font color=red>附註:</font> 在執行升級之前, 請確定您的韌體是正確的。假如您不知道該用哪種韌體, 請與廠商聯絡以尋求技術上的支援。";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "點擊「<b>確定</b>」 以回復出廠的預設值。<BR><FONT CLASS=F1 COLOR=#FF3300><B>請注意! 所有目前的設定值都將被抹除。</B></FONT>";
textArray1[iIndex++] = "回復出廠值, 不包括 TCP/IP 設定. (推薦)";
textArray1[iIndex++] = "在它重新啟動後, 您仍然能用目前的 IP 連到它。";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;確定&nbsp;&nbsp;" onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="韌體升級";
textArray2[iIndex++]="選擇檔案:";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="韌體升級" onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="本頁可以讓您重新啟動印表伺服器。<br>";
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>重新啟動印表伺服器</B></FONT><br><br>你想要重新啟動印表伺服器嗎?<br><br>";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;確定&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

	//restart.htm
textArray4[iIndex++] = "重新啟動中 ...";
textArray4[iIndex++] = "請等待印表伺服器重新啟動。";
iIndex = 0;
	//urestart.htm
textArray5[iIndex++] = "升級成功 !";
textArray5[iIndex++] = "韌體升級成功後, 印表伺服器將會自動重新啟動。請等待印表伺服器重新啟動。";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "回復為出廠的設定值中 ...";
textArray6[iIndex++] = "回復為出廠的設定值後, 印表伺服器將會自動重新啟動。<BR><BR>請等待印表伺服器重新啟動。";
iIndex = 0;

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=big5">');
}

// out of Traditional Chinese
