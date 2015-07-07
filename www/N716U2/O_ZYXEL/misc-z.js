

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Chinese
tabArray=['狀態','設定','其他','重新啟動','出廠預設值','升級韌體'];

	//upgrade.htm
headArray[iIndex++] ="<br>您可以使用此頁升級列印伺服器的韌體。<br>注意：進行升級前請確認韌體是否正確。如果不確定應該使用何種韌體，請洽當地經銷商取得技術支援。";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "按一下 <b>出廠預設值</b> 再按一下 <b>確定</b>，可重新載入列印伺服器所有的預設設定。警告！目前設定已全部清除。";
textArray1[iIndex++] = "按一下 <b>升級韌體</b> 瀏覽韌體目錄，重新載入具備新韌體的列印伺服器。";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;確定&nbsp;&nbsp;" onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="升級韌體";
textArray2[iIndex++]="選取韌體目錄和檔案：";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="升級韌體" onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="您可以使用此頁重新啟動列印伺服器。<br>";
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>重新啟動列印伺服器</B></FONT><br><br>您要立即儲存設定並重新啟動列印伺服器嗎？<br><br>";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;確定&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

	//restart.htm
textArray4[iIndex++] = "重新啟動中 ...";
textArray4[iIndex++] = "列印伺服器重新啟動中，請稍待。";
iIndex = 0;
	//urestart.htm
textArray5[iIndex++] = "成功完成升級！";
textArray5[iIndex++] = "韌體升級後，列印伺服器會自動重新啟動，請稍待。";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "載入出廠預設值 ...";
textArray6[iIndex++] = "載入預設設定後，列印伺服器會自動重新啟動。<br><br>列印伺服器重新啟動時，系統會將您重新導向 Status (狀態) 頁面。";
iIndex = 0;
// out of English
