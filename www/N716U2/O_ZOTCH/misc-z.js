

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Chinese
//tabArray=['PU211 USB 埠印表伺服器','狀態','設定','其它','重新啟動','回復出廠值','韌體升級'];
tabArray=['回復出廠值','韌體升級'];

	//upgrade.htm
headArray[iIndex++] ="<br>本頁可以讓您升級印表伺服器的韌體。<BR><font color=red>附註:</font> 在執行升級之前, 請確定您的韌體是正確的。假如您不知道該用哪種韌體, 請與廠商聯絡以尋求技術上的支援。";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "點擊「<b>確定</b>」 以回復出廠的預設值。<BR><FONT CLASS=F1 COLOR=#FF3300><B>請注意! 所有目前的設定值都將被抹除。</B></FONT>";
textArray1[iIndex++] = "回復出廠值, 不包括 TCP/IP 設定. (推薦)";
textArray1[iIndex++] = "在它重新啟動後, 您仍然能用目前的 IP 連到它。";
textArray1[iIndex++] = "回復出廠值, 包括 TCP/IP 設定.";
textArray1[iIndex++] = "在它重新啟動後, 您無法用目前的 IP 連到它。";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;確定&nbsp;&nbsp;" onClick="return SaveIPSetting(';
// Begin don't translate
textArray1[iIndex++] = "'DRESTART.HTM');";
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
textArray6[iIndex++] = " 回復為出廠的設定值後, 印表伺服器將會自動重新啟動。<BR><BR>請等待印表伺服器重新啟動。";
iIndex = 0;

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=big5">');
}

// Title or Model Name
function TitleModelName()
{
	document.write('<title>PU211 USB 埠印表伺服器</title>');
}

// MM_preloadImages
function BodyPreloadImages2()
{
	document.write("<body onload=OnLoadTasks()>");
}

function OnLoadTasks()
{
	MM_preloadImages('imgzhtw/MenuBtn-CS-setup2.gif','imgzhtw/MenuBtn-CS-other2.gif','imgzhtw/MenuBtn-CS-restart2.gif');
	GoToLastPage();
}

// mainView-Title
function MainViewTitle()
{
	document.write('<tr><td><img src="images/mainView-2.gif" width="301" height="32" /></td>');
	document.write('<td><img src="imgzhtw/mainView-Title.gif" width="431" height="32">');
	document.write('</td></tr>');
}

// Row MenuBtn
function RowMenuBtn()
{
	document.write("<td><a href=SYSTEM.HTM target=_parent onmouseover=MM_swapImage('Image14','','imgzhtw/MenuBtn-CS-status2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgzhtw/MenuBtn-CS-status1.gif name=Image14 width=93 height=36 border=0 id=Image14></a></td>");
	document.write("<td><a href=CSYSTEM.HTM target=_parent onmouseover=MM_swapImage('Image15','','imgzhtw/MenuBtn-CS-setup2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgzhtw/MenuBtn-CS-setup1.gif name=Image15 width=93 height=36 border=0 id=Image15></a></td>");
	document.write('<td><img src="imgzhtw/MenuBtn-CS-other3.gif" width="92" height="36" /></td>');
	document.write("<td><a href=RESET.HTM target=_parent onmouseover=MM_swapImage('Image16','','imgzhtw/MenuBtn-CS-restart2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgzhtw/MenuBtn-CS-restart1.gif name=Image16 width=93 height=36 border=0 id=Image16></a></td>");
}

function RowMenuBtn4()
{
	document.write("<td><a href=SYSTEM.HTM target=_parent onmouseover=MM_swapImage('Image14','','imgzhtw/MenuBtn-CS-status2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgzhtw/MenuBtn-CS-status1.gif name=Image14 width=93 height=36 border=0 id=Image14></a></td>");
	document.write("<td><a href=CSYSTEM.HTM target=_parent onmouseover=MM_swapImage('Image15','','imgzhtw/MenuBtn-CS-setup2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgzhtw/MenuBtn-CS-setup1.gif name=Image15 width=93 height=36 border=0 id=Image15></a></td>");
	document.write("<td><a href=DEFAULT.HTM target=_parent onmouseover=MM_swapImage('Image16','','imgzhtw/MenuBtn-CS-other2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgzhtw/MenuBtn-CS-other1.gif name=Image16 width=93 height=36 border=0 id=Image16></a></td>");
	document.write('<td><img src="imgzhtw/MenuBtn-CS-restart3.gif" width="92" height="36" /></td>');
}

// out of Chinese
