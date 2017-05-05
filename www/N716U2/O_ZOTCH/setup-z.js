
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Chinese
//tabArray=['PU211 USB 埠印表伺服器','狀態','設定','其它','重新啟動','系統','TCP/IP','進階','NetWare','AppleTalk','SNMP','SMB','',''];
tabArray=['系統','TCP/IP','進階','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>本頁可以讓你修改此印表伺服器的基本設定。<br>";
//cprinter.htm
headArray[iIndex++] = "<BR>本頁可以讓你啟動或取消印表伺服器連接埠的雙向列印功能。";
//ctcpip.htm
headArray[iIndex++] = "<BR>本頁可以讓你修改此印表伺服器 TCP/IP 的設定。";
//cservices.htm
headArray[iIndex++] = "<BR>本頁可以讓你修改此印表伺服器進階相關的設定項目。";
//cnetware.htm
headArray[iIndex++] = "<BR>本頁可以讓你修改此印表伺服器 NetWare 相關的設定項目。";
//capple.htm 
headArray[iIndex++] = "<BR>本頁可以讓你修改此印表伺服器 AppleTalk 相關的設定項目。<br>";
//csnmp.htm
headArray[iIndex++] = "<BR>本頁可以讓你修改此印表伺服器的 SNMP 的設定。";
//csmp.htm
headArray[iIndex++] = "<BR>本頁可以讓你修改此印表伺服器的微軟網路芳鄰印表機分享設定。";
iIndex = 0;

//csystem.htm
textArray0[iIndex++]="E-mail 警示設定";
textArray0[iIndex++]="E-mail 警示:";
textArray0[iIndex++]="停用";
textArray0[iIndex++]="啟用";
textArray0[iIndex++]="外寄郵件伺服器 IP 地址:";
textArray0[iIndex++]="管理者 E-mail 信箱:";
textArray0[iIndex++]="系統設定";
textArray0[iIndex++]="裝置名稱 :";
textArray0[iIndex++]="連絡人 :";
textArray0[iIndex++]="裝置位置 :";
textArray0[iIndex++]="系統管理者密碼";
textArray0[iIndex++]="管理者帳號 :";
textArray0[iIndex++]="admin";
textArray0[iIndex++]="密碼 :";
textArray0[iIndex++]="密碼確認 :";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="儲存並重新啟動" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
//CPRINTER.htm
textArray1[iIndex++]="平行埠 - 雙向列印設定";
textArray1[iIndex++]="連接埠 :";
textArray1[iIndex++]="停用";
textArray1[iIndex++]="自動偵測";
textArray1[iIndex++]="列印速度 :";
textArray1[iIndex++]="快";
textArray1[iIndex++]="中等";
textArray1[iIndex++]="慢";
textArray1[iIndex++]="若印表機為雷射印表機∕影印機，建議設定為快。";
textArray1[iIndex++]="若印表機為點矩陣式印表機，建議設定為慢。";
//textArray1[iIndex++]="LPR 佇列名稱 :";
//textArray1[iIndex++]="(最長 12 個英數字元, 不能有空白字元)";
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
textArray2[iIndex++]="IP 位址 :";
textArray2[iIndex++]="子網路遮罩 :";
textArray2[iIndex++]="預設閘道器 :";
//crandvoo.htm
textArray2[iIndex++]="Rendezvous 設定";
textArray2[iIndex++]="Rendezvous 設定 :";
//textArray2[iIndex++]="停用";
//textArray2[iIndex++]="啟用";
textArray2[iIndex++]="服務名稱 :";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="儲存並重新啟動" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="AppleTalk 設定";
textArray3[iIndex++]="AppleTalk 服務 :";
textArray3[iIndex++]="停用";
textArray3[iIndex++]="啟用";
textArray3[iIndex++]="AppleTalk 區域名稱 :";
textArray3[iIndex++]="連接埠名稱 :";
textArray3[iIndex++]="連接埠";
textArray3[iIndex++]="印表機形式 :";
textArray3[iIndex++]="資料格式 :";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="儲存並重新啟動" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
//CSNMP.htm
textArray4[iIndex++]="SNMP 群體設定";
textArray4[iIndex++]="SNMP 服務 :";
textArray4[iIndex++]="停用";
textArray4[iIndex++]="啟用";
textArray4[iIndex++]="支援 HP WebJetAdmin :";
textArray4[iIndex++]="停用";
textArray4[iIndex++]="啟用";
textArray4[iIndex++]="群體名稱 1 :";
textArray4[iIndex++]="群體權限 :";
textArray4[iIndex++]="只能讀";
textArray4[iIndex++]="讀寫皆可";
textArray4[iIndex++]="群體名稱 2 :";
textArray4[iIndex++]="群體權限 :";
textArray4[iIndex++]="只能讀";
textArray4[iIndex++]="讀寫皆可";
textArray4[iIndex++]="SNMP 陷阱設定";
textArray4[iIndex++]="使用陷阱補抓 :";
textArray4[iIndex++]="停用";
textArray4[iIndex++]="啟用";
textArray4[iIndex++]="傳送確認陷阱 :";
textArray4[iIndex++]="停用";
textArray4[iIndex++]="啟用";
textArray4[iIndex++]="陷阱目標 IP 位址 1 :";
textArray4[iIndex++]="陷阱目標 IP 位址 2 :";
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
textArray7[iIndex++]="IP 地址錯誤";
textArray7[iIndex++]="子網路遮罩錯誤";
textArray7[iIndex++]="閘道器 IP 地址錯誤";
textArray7[iIndex++]="輪詢週期值錯誤";
textArray7[iIndex++]="印表伺服器名稱錯誤";
textArray7[iIndex++]="NetWare 檔案伺服器名稱錯誤";
textArray7[iIndex++]="找不到 DHCP/BOOTP 伺服器";
textArray7[iIndex++]="SNMP 陷阱 IP 地址錯誤";
textArray7[iIndex++]="密碼錯誤";
textArray7[iIndex++]="韌體有問題或升級失敗";
textArray7[iIndex++]="匯入設定檔 CFG.bin 失敗";
textArray7[iIndex++]="";
textArray7[iIndex++]="回到上一頁";
iIndex = 0;

// CNETWARE.htm
textArray8[iIndex++]="基本設定";
textArray8[iIndex++]="印表伺服器名稱 :";
textArray8[iIndex++]="輪詢時間 :";
textArray8[iIndex++]="&nbsp;秒 (最小: 3 秒, 最大: 29 秒)";
textArray8[iIndex++]="登入 NetWare 的密碼 :";
textArray8[iIndex++]="NetWare NDS 設定";
textArray8[iIndex++]="使用 NDS 模式 :";
textArray8[iIndex++]="停用";
textArray8[iIndex++]="啟用";
textArray8[iIndex++]="NDS Tree 名稱 :";
textArray8[iIndex++]="NDS Context 名稱 :";
textArray8[iIndex++]="NetWare Bindery 設定";
textArray8[iIndex++]="使用 Bindery 模式 :";
textArray8[iIndex++]="停用";
textArray8[iIndex++]="啟用";
textArray8[iIndex++]="檔案伺服器名稱 :";
textArray8[iIndex++]='<input type="button" value="儲存並重新啟動" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.htm
textArray9[iIndex++]="工作群組";
textArray9[iIndex++]="SMB 服務 :";
textArray9[iIndex++]="停用";
textArray9[iIndex++]="啟用";
textArray9[iIndex++]="名稱 :";
textArray9[iIndex++]="印表機共用名稱";
textArray9[iIndex++]="連接埠 :";
textArray9[iIndex++]='<input type="button" value="儲存並重新啟動" onClick="return CheckSMB(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;

//CSERVICES.htm
textArray10[iIndex++]="列印方式";
textArray10[iIndex++]="使用 LPR/LPD 列印 :";
textArray10[iIndex++]="停用";
textArray10[iIndex++]="啟用";
textArray10[iIndex++]="使用 AppleTalk 列印 :";
textArray10[iIndex++]="停用";
textArray10[iIndex++]="啟用";
textArray10[iIndex++]="使用 IPP 列印 :";
textArray10[iIndex++]="停用";
textArray10[iIndex++]="啟用";
textArray10[iIndex++]="使用 SMB 列印 :";
textArray10[iIndex++]="停用";
textArray10[iIndex++]="啟用";
textArray10[iIndex++]="服務";
textArray10[iIndex++]="Telnet :";
textArray10[iIndex++]="停用";
textArray10[iIndex++]="啟用";
textArray10[iIndex++]="SNMP :";
textArray10[iIndex++]="停用";
textArray10[iIndex++]="啟用";
textArray10[iIndex++]="E-mail 和結束頁警示 :";
textArray10[iIndex++]="停用";
textArray10[iIndex++]="啟用";
textArray10[iIndex++]="HTTP :";
textArray10[iIndex++]="停用";
textArray10[iIndex++]="啟用";
textArray10[iIndex++]="請注意: 在您停用 HTTP 後, 只能用 Reset 鍵啟用它.";
textArray10[iIndex++]="而且, 停用 HTTP 也會一併停用 IPP 列印.";
textArray10[iIndex++]='<input type="button" value="儲存並重新啟動" onClick="return SaveServices(';
// Begin don't translate
textArray10[iIndex++]="'RESTART.HTM');";
textArray10[iIndex++]='">';
iIndex = 0;

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=big5">');
}

// functions
// CSYSTEM.HTM
function CheckPwd(szURL)
{
 	if(document.CSYSTEM.SetupPWD.value != document.CSYSTEM.ConfirmPWD.value && document.CSYSTEM.SetupPWD.value != "ZO__I-SetupPassword" )
	{
		alert("系統管理者的密碼與密碼確認不符 !");
		return false;
	}
	document.CSYSTEM.action=szURL;
	document.CSYSTEM.submit();
	return false;
}

// CPRINTER.HTM
function CheckLPRQueueName(szURL)
{
	var ch;
	var lprname1 = document.forms[0].LPRQueue1.value;

	if (document.forms[0].LPRQueue1.value == '')
	{
		alert("錯誤! LPR 佇列名稱不能空白!");
	 	return false; 
	}
	else
	{
		for(var i = 0 ; i < lprname1.length ; i++ )
		{
	    	ch = lprname1.charAt(i).toUpperCase();
	
			if(!((ch >= '0' && ch <= '9') ||
				(ch >= 'A' && ch <= 'Z') || 
				(ch >= 'a' && ch <= 'z') || 
				(ch == '-') || (ch == '_')))
			{
				alert ("LPR 佇列名稱 " + lprname1 + " 的 " + ch + " 為無效字元.");
				document.forms[0].LPRQueue1.focus();
				return false;
			}
		}

		document.forms[0].action=szURL;
		document.forms[0].submit(); 
		return false; 
	}
}

// CTCPTP.HTM

// CNETWARE.HTM

// CAPPLE.HTM

// CSNMP.HTM

// CSMB.HTM
function CheckSMB(szURL)
{
	if(document.forms[0].SMBWorkGroup.value == '')
	{
		alert("錯誤! 工作群組名稱不能空白!");
		return false;
	}
	else
	{
		if(document.forms[0].SMBPrint1.value == '')
		{		
			alert("錯誤! SMB 共享印表機名稱不能空白!"); 
		 	return false; 
		}
		else
		{
			document.forms[0].action=szURL;
			document.forms[0].submit(); 
			return false;
		}
	}
}


// Title or Model Name
function TitleModelName()
{
	document.write('<title>PU211 USB 埠印表伺服器</title>');
}

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=big5">');
}

// MM_preloadImages
function BodyPreloadImages()
{
	document.write("<body onload=MM_preloadImages('imgzhtw/MenuBtn-CS-setup2.gif','imgzhtw/MenuBtn-CS-other2.gif','imgzhtw/MenuBtn-CS-restart2.gif')>");
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
	document.write('<td><img src="imgzhtw/MenuBtn-CS-setup3.gif" width="92" height="36" /></td>');
	document.write("<td><a href=DEFAULT.HTM target=_parent onmouseover=MM_swapImage('Image15','','imgzhtw/MenuBtn-CS-other2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgzhtw/MenuBtn-CS-other1.gif name=Image15 width=93 height=36 border=0 id=Image15></a></td>");
	document.write("<td><a href=RESET.HTM target=_parent onmouseover=MM_swapImage('Image16','','imgzhtw/MenuBtn-CS-restart2.gif',1) onmouseout=MM_swapImgRestore()><img src=imgzhtw/MenuBtn-CS-restart1.gif name=Image16 width=93 height=36 border=0 id=Image16></a></td>");
}


// out of Chinese
