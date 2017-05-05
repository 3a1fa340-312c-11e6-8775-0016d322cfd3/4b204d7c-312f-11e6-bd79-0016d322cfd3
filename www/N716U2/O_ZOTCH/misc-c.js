

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Simplified Chinese
//tabArray=['PU211 USB 口打印服务器','状态','设置','其它','重启','回到默认值','升级固件'];
tabArray=['回到默认值','升级固件'];

	//upgrade.htm
headArray[iIndex++] ="<br>此屏幕允许您升级打印服务器的固件。<br>注意: 在您继续之前请确认打印服务器的固件是正确的。如果您不能确认，请与当地供应商联系。";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "点击 “<b>回到默认值</b>” 然后点击 “<b>确定</b>” 以回到默认值。 <BR><FONT CLASS=F1 COLOR=#FF3300><B>警告! 打印服务器内配置值将全部被清除。</B></FONT><br><br>";
textArray1[iIndex++] = "回到默认设置, 不包括 TCP/IP 设置. (推荐)";
textArray1[iIndex++] = "在它重启后, 您仍然能用当前的 IP 地址访问.";
textArray1[iIndex++] = "回到默认设置, 包括 TCP/IP 设置.";
textArray1[iIndex++] = "在它重启后, 您无法用当前的 IP 地址访问.";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;确定&nbsp;&nbsp;" onClick="return SaveIPSetting(';
// Begin don't translate
textArray1[iIndex++] = "'DRESTART.HTM');";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="升级固件";
textArray2[iIndex++]="选择文件 :";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="升级固件" onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="此屏幕允许您重新启动打印服务器。<br>";
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>重新启动打印服务器</B></FONT><br><br>您确定现在要重新启动打印服务器吗 ?<br><br>";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;确定&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

	//restart.htm
textArray4[iIndex++] = "重新启动...";
textArray4[iIndex++] = "请稍等";
iIndex = 0;
	//urestart.htm
textArray5[iIndex++] = "升级成功!";
textArray5[iIndex++] = "升级成功后，打印服务器自动重新启动。请稍等";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "重新加载出厂默认值...";
textArray6[iIndex++] = "回复出厂默认值后，此打印服务器自动重新启动。<BR><BR>重新启动后, 打印服务器会带您回到主页。";
iIndex = 0;

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=gb2312">');
}

// Title or Model Name
function TitleModelName()
{
	document.write('<title>PU211 USB 口打印服务器</title>');
}

// MM_preloadImages
function BodyPreloadImages2()
{
	document.write("<body onload=OnLoadTasks()>");
}

function OnLoadTasks()
{
	MM_preloadImages('imgzhcn/MenuBtn-C-setup2.gif','imgzhcn/MenuBtn-C-other2.gif','imgzhcn/MenuBtn-C-restart2.gif');
	GoToLastPage();
}

// mainView-Title
function MainViewTitle()
{
	document.write('<tr><td><img src="images/mainView-2.gif" width="301" height="32" /></td>');
	document.write('<td><img src="imgzhcn/mainView-Title-C.gif" width="431" height="32">');
	document.write('</td></tr>');
}

// Row MenuBtn
function RowMenuBtn()
{
	document.write("<td><a href=SYSTEM.HTM target=_parent onmouseover=MM_swapImage('Image14','','imgzhcn/MenuBtn-C-status2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgzhcn/MenuBtn-C-status1.gif name=Image14 width=93 height=36 border=0 id=Image14></a></td>");
	document.write("<td><a href=CSYSTEM.HTM target=_parent onmouseover=MM_swapImage('Image15','','imgzhcn/MenuBtn-C-setup2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgzhcn/MenuBtn-C-setup1.gif name=Image15 width=93 height=36 border=0 id=Image15></a></td>");
	document.write('<td><img src="imgzhcn/MenuBtn-C-other3.gif" width="92" height="36" /></td>');
	document.write("<td><a href=RESET.HTM target=_parent onmouseover=MM_swapImage('Image16','','imgzhcn/MenuBtn-C-restart2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgzhcn/MenuBtn-C-restart1.gif name=Image16 width=93 height=36 border=0 id=Image16></a></td>");
}

function RowMenuBtn4()
{
	document.write("<td><a href=SYSTEM.HTM target=_parent onmouseover=MM_swapImage('Image14','','imgzhcn/MenuBtn-C-status2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgzhcn/MenuBtn-C-status1.gif name=Image14 width=93 height=36 border=0 id=Image14></a></td>");
	document.write("<td><a href=CSYSTEM.HTM target=_parent onmouseover=MM_swapImage('Image15','','imgzhcn/MenuBtn-C-setup2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgzhcn/MenuBtn-C-setup1.gif name=Image15 width=93 height=36 border=0 id=Image15></a></td>");
	document.write("<td><a href=DEFAULT.HTM target=_parent onmouseover=MM_swapImage('Image16','','imgzhcn/MenuBtn-C-other2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgzhcn/MenuBtn-C-other1.gif name=Image16 width=93 height=36 border=0 id=Image16></a></td>");
	document.write('<td><img src="imgzhcn/MenuBtn-C-restart3.gif" width="92" height="36" /></td>');
}

// out of Simplified Chinese
