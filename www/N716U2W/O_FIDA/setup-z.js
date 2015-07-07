
//vaiable
tabindex = 0;
textindex = 0;
textindex10 = 0;		// CWLAN.htm Basic Settings
textindex11 = 0;		// CWLAN.htm Advanced Settings
textindex12 = 0;		// CWLAN.htm Site Survey
var iIndex = 0;

//Language : Traditional Chinese
tabArray=['<IMG SRC=images/logo.jpg>','Status','Setup','Misc','Restart','System','Wireless','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];
//tabArray=['無線/有線雙介面印表伺服器','狀態','設定','其它','重新啟動','系統','無線','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>本頁可以讓你修改此印表伺服器的基本設定。<br>";
//cwlan.htm
headArray[iIndex++] = "<BR>本頁可以讓你修改此印表伺服器無線相關設定。";
//ctcpip.htm
headArray[iIndex++] = "<BR>本頁可以讓你修改此印表伺服器 TCP/IP 的設定。";
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
textArray1[iIndex++]="連接埠 1 :";
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
textArray8[iIndex++]="<option>找不到檔案伺服器!</option>";
textArray8[iIndex++]='<input type="button" value="儲存並重新啟動" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.htm
textArray9[iIndex++]="工作群組";
textArray9[iIndex++]="名稱 :";
textArray9[iIndex++]="印表機共用名稱";
textArray9[iIndex++]="連接埠 :";
textArray9[iIndex++]='<input type="button" value="儲存並重新啟動" onClick="return CheckSMB(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;

// CWLAN.htm Basic Settings
textArray10[iIndex++]="基本設定";
textArray10[iIndex++]="網路類型 :";
textArray10[iIndex++]="SSID :";
textArray10[iIndex++]="頻道 :";
//textArray10[iIndex++]="無線地區 :";
//textArray10[iIndex++]="1 - 11, 美國, 加拿大, 烏克蘭, 中國";
//textArray10[iIndex++]="1 - 13, 歐洲 (ETSI), 法國和西班牙除外";
//textArray10[iIndex++]="10 - 13, 法國, 新加坡";
//textArray10[iIndex++]="10 - 11, 西班牙, 墨西哥";
//textArray10[iIndex++]="變更無線地區設置必須遵守當地電信法令.";
//textArray10[iIndex++]="請儲存並重新啟動以套用此項變更.";
//textArray10[iIndex++]='<input type="button" value="儲存並重新啟動" onClick="return SaveSettingCWLAN(';
// Begin don't translate
//textArray10[iIndex++]="'RESTART.HTM');";
//textArray10[iIndex++]='">';
textArray10[iIndex++]="傳輸速率 :";
textArray10[iIndex++]="自動偵測";
textArray10[iIndex++]="無線模式 :";
textArray10[iIndex++]="自動選擇 B/G";
textArray10[iIndex++]="只使用 B";
textArray10[iIndex++]="只使用 G";
textArray10[iIndex++]="自動選擇 B/G/N";
iIndex = 0;
// End don't translate

//CWLAN.htm Advanced Settings
textArray11[iIndex++]="進階設定";
textArray11[iIndex++]="安全性類型";
textArray11[iIndex++]="不提高安全性";
textArray11[iIndex++]="WEP";
textArray11[iIndex++]="金鑰索引 :";
textArray11[iIndex++]="金鑰加密 :";
textArray11[iIndex++]="64 位元 (十六進位長度 10)";
textArray11[iIndex++]="64 位元 (英數字長度 5)";
textArray11[iIndex++]="128 位元 (十六進位長度 26)";
textArray11[iIndex++]="128 位元 (英數字長度 13)";
textArray11[iIndex++]="金鑰 1 :";
textArray11[iIndex++]="金鑰 2 :";
textArray11[iIndex++]="金鑰 3 :";
textArray11[iIndex++]="金鑰 4 :";
textArray11[iIndex++]="網路驗證 :";
textArray11[iIndex++]="開放式系統";
textArray11[iIndex++]="共享金鑰";
//textArray11[iIndex++]="Auto";
textArray11[iIndex++]="WPA-PSK";
textArray11[iIndex++]="WPA2-PSK";
textArray11[iIndex++]="金鑰加密 :";
textArray11[iIndex++]="網路安全金鑰 :";
textArray11[iIndex++]="英數字長度 8 到 63 (0 到 9, A 到 Z, a 到 z), 或十六進位長度 64 (0 到 9, A 到 F)";
// Translate                                  only "Save & Restart" is to be translated
textArray11[iIndex++]='<input type="button" value="儲存並重新啟動" onClick="return SaveSettingCWLAN(';
// Begin don't translate
textArray11[iIndex++]="'RESTART.HTM');";
textArray11[iIndex++]='">';
iIndex = 0;
// End don't translate

//CWLAN.htm Site Survey
textArray12[iIndex++]="尋找可用的無線網路";
textArray12[iIndex++]="SSID";
textArray12[iIndex++]="MAC 地址";
textArray12[iIndex++]="頻道";
textArray12[iIndex++]="網路類型";
textArray12[iIndex++]="安全性";
textArray12[iIndex++]="信號強度(dBm)";
textArray12[iIndex++]='<input name="refresh" onclick="return OnSiteSurvey()" type=button value=" 更新 " style="font-family: Verdana; font-size: 11px;">';
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
 	if(document.forms[0].SetupPWD.value != document.forms[0].ConfirmPWD.value && document.forms[0].SetupPWD.value != "ZO__I-SetupPassword" )
	{
		alert("系統管理者的密碼與密碼確認不符 !");
		return false;
	}
	document.forms[0].action=szURL;
	document.forms[0].submit();
	return false;
}

// CPRINTER.HTM
function CheckLPRQueueName(szURL)
{
	if ((document.forms[0].LPRQueue1.value == '')
		||(document.forms[0].LPRQueue2.value == '')
		||(document.forms[0].LPRQueue3.value == ''))
	{
		alert("錯誤! LPR 佇列名稱不能空白!");
	 	return false; 
	}
	else
	{
		if ((document.forms[0].LPRQueue1.value == document.forms[0].LPRQueue2.value)
			||(document.forms[0].LPRQueue2.value == document.forms[0].LPRQueue3.value)
			||(document.forms[0].LPRQueue1.value == document.forms[0].LPRQueue3.value))
	 	{
	 		alert("錯誤! LPR 佇列名稱不能重複!");
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

// CWLAN.HTM
//-----------------------------------------------------------------------------
function OnSiteSurvey()
{
	newwindow=window.open("WLANTMP0.HTM","","toolbar=0,location=0,directories=0,status=0,menubar=0,width=700,height=400,scrollbars=1");
	return false;
}

//-----------------------------------------------------------------------------
function validate_Preshared()
{	
	var f = document.myform;
	
	if( !CheckWPASharedKey(f) )
		return false;
	else
		return true;
}

//-----------------------------------------------------------------------------
function CheckWPASharedKey(f)
{
	var k = f.WPA_Pass.value;
	
	if( k == '' )
	{
		alert("請輸入 WPA-PSK 或 WPA2-PSK 共享金鑰.");
		return false;
	}
	
	if( k.length == 64 )
	{
		if( !CheckHexKey(k) )
			return false;
	}
	else if( k.length < 8 || k.length > 63 )
	{
		alert("英數字長度 8 到 63. 只允許 0 到 9, A 到 Z, a 到 z 這些字!");
		return false;
	}
	
	return true;
}

//-----------------------------------------------------------------------------
function CheckWPASharedKey(f)
{
	var k = f.WPA_Pass.value;
	
	if( k == '' )
	{
		alert("請輸入 WPA-PSK 或 WPA2-PSK 共享金鑰.");
		return false;
	}
	
	if( k.length == 64 )
	{
		if( !CheckHexKey(k) )
			return false;
	}
	else if( k.length < 8 || k.length > 63 )
	{
		alert("英數字長度 8 到 63. 只允許 0 到 9, A 到 Z, a 到 z 這些字!");
		return false;
	}
	
	return true;
}

//-----------------------------------------------------------------------------
function CheckHexKey(k)
{
	var iln, ch;

	for ( iln = 0; iln < k.length; iln++ )
	{
    	ch = k.charAt(iln).toLowerCase();
		
	  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') )
			continue;
	  	else 
		{
	    	alert("您輸入的網路金鑰裡的 " + ch + " 不是有效字, 位於第 " + k);
	    	return false;
	  	}
	}

	return true;
}

//-----------------------------------------------------------------------------
function SaveSettingCWLAN(szURL)
{
	var f = document.myform;
	var iln, ch;

	// Basic Settings
	// Wireless Mode
	
	// Advanced Settings
	// Security Mode
	if( f.security_mode[1].checked )
	{
		// WEP 64-bit or WEP 128-bit
		var hs;
	
		var iWEP_sel;			// Key Index
		var iWEP_type;			// WEP Encryption
		
		if (f.wep_sel[0].checked)
		{
			iWEP_sel = 0;
			
			if(f.wep_key1.value == '')
			{
				alert("WEP 金鑰不能空白!");
				return false;
			}
		}
		if (f.wep_sel[1].checked)
		{
			iWEP_sel = 1;
			
			if(f.wep_key2.value == '')
			{
				alert("WEP 金鑰不能空白!");
				return false;
			}
		}
		if (f.wep_sel[2].checked)
		{
			iWEP_sel = 2;
			
			if(f.wep_key3.value == '')
			{
				alert("WEP 金鑰不能空白!");
				return false;
			}
		}
		if (f.wep_sel[3].checked)
		{
			iWEP_sel = 3;
			
			if(f.wep_key4.value == '')
			{
				alert("WEP 金鑰不能空白!");
				return false;
			}
		}
		
		f.WLWEPKeySel.value = iWEP_sel;

		iWEP_type = f.wep_type.selectedIndex;
		// [0]: 64-bit 10 hex digits	[1]: 64-bit 5 characters
		// [2]: 128-bit 26 hex digits	[3]: 128-bit 13 characters

		// 64-bit hex or 128-bit hex
		if((iWEP_type == 0) || (iWEP_type == 2))
		{
			f.WLWEPFormat.value = 1;	// Hexadecimal

			switch(iWEP_sel)
			{
			case 0:			// WEP key 1
				hs = convertHexString (document.myform.wep_key1.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP 金鑰 1\" 格式錯誤!");
					return false;
				}
				break;
			case 1:			// WEP key 2
				hs = convertHexString (document.myform.wep_key2.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP 金鑰 2\" 格式錯誤!");
					return false;
				}
				break;
			case 2:			// WEP key 3
				hs = convertHexString (document.myform.wep_key3.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP 金鑰 3\" 格式錯誤!");
					return false;
				}
				break;
			case 3:			// WEP key 4
				hs = convertHexString (document.myform.wep_key4.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP 金鑰 4\" 格式錯誤!");
					return false;
				}
				break;
			default:		// WEP key 1
				hs = convertHexString (document.myform.wep_key1.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP 金鑰 1\" 格式錯誤!");
					return false;
				}
			}
		}	// end of if((iWEP_type == 0) || (iWEP_type == 2))
		
		// 64-bit Alphanumeric or 128-bit Alphanumeric
		if((iWEP_type == 1) || (iWEP_type == 3))
		{
			f.WLWEPFormat.value = 0;		// Alphanumeric
			
			switch(iWEP_sel)
			{
			case 0:			// WEP key 1
				for ( iln = 0; iln < f.wep_key1.value.length; iln++ )
				{
			    	ch = f.wep_key1.value.charAt(iln);
					
				  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '!') || (ch == '?') )
				  		continue;
				  	else
				  	{
				  		alert ("\"WEP 金鑰 1\" 裡有無效字, 只能寫 0 到 9, a 到 z, A 到 Z!");
				  		return false;
				  	}
				}
				break;
			case 1:			// WEP key 2
				for ( iln = 0; iln < f.wep_key2.value.length; iln++ )
				{
			    	ch = f.wep_key2.value.charAt(iln);

				  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '!') || (ch == '?') )
				  		continue;
				  	else
				  	{
				  		alert ("\"WEP 金鑰 2\" 裡有無效字, 只能寫 0 到 9, a 到 z, A 到 Z!");
				  		return false;
				  	}
				}
				break;
			case 2:			// WEP key 3
				for ( iln = 0; iln < f.wep_key3.value.length; iln++ )
				{
			    	ch = f.wep_key3.value.charAt(iln);
					
				  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '!') || (ch == '?') )
				  		continue;
				  	else
				  	{
				  		alert ("\"WEP 金鑰 3\" 裡有無效字, 只能寫 0 到 9, a 到 z, A 到 Z!");
				  		return false;
				  	}
				}
				break;
			case 3:			// WEP key 4
				for ( iln = 0; iln < f.wep_key4.value.length; iln++ )
				{
			    	ch = f.wep_key4.value.charAt(iln);
					
				  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '!') || (ch == '?') )
				  		continue;
				  	else
				  	{
				  		alert ("\"WEP 金鑰 4\" 裡有無效字, 只能寫 0 到 9, a 到 z, A 到 Z!");
				  		return false;
				  	}
				}
				break;
			default:		// WEP key 1
				for ( iln = 0; iln < f.wep_key1.value.length; iln++ )
				{
			    	ch = f.wep_key1.value.charAt(iln);
					
				  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '!') || (ch == '?') )
				  		continue;
				  	else
				  	{
				  		alert ("\"WEP 金鑰 1\" 裡有無效字, 只能寫 0 到 9, a 到 z, A 到 Z!");
				  		return false;
				  	}
				}
			}
		}
		
		f.WLWEPType.value = 1;	// default 1: WEP 64-bit

		if((f.wep_type.options[0].selected) || (f.wep_type.options[1].selected))
		{
			f.WLWEPType.value = 1;	// WEP 64-bit

			f.WLWEPKey1.value = f.wep_key1.value;
			f.WLWEPKey2.value = f.wep_key2.value;
			f.WLWEPKey3.value = f.wep_key3.value;
			f.WLWEPKey4.value = f.wep_key4.value;
		}

		if((f.wep_type.options[2].selected) || (f.wep_type.options[3].selected))
		{
			// WEP 128-bit
			f.WLWEPType.value = 2;
			
			f.WLWEP128Key.value = f.wep_key1.value;
			f.WLWEP128Key2.value = f.wep_key2.value;
			f.WLWEP128Key3.value = f.wep_key3.value;
			f.WLWEP128Key4.value = f.wep_key4.value;
		}

		// Authentication
		if(f.wep_authmode[0].selected)
			f.WLAuthType.value = 1;		// Open System
		if(f.wep_authmode[1].selected)
			f.WLAuthType.value = 2;		// Shared Key
	}
	else if( f.security_mode[2].checked )
	{
		// WPA-PSK
		f.WLAuthType.value = 4;

		if(!validate_Preshared())
			return false;
	}
	else if( f.security_mode[3].checked )
	{
		// WPA2-PSK
		f.WLAuthType.value = 5;

		if(!validate_Preshared())
			return false;
	}
	else
	{
		// Disabled
		f.WLWEPType.value = 0;
		f.WLAuthType.value = 1;
	}

	document.forms[0].action=szURL;
	document.forms[0].submit();
	return false; 
}

function checkPreSharedKey(szURL)
{
	var theForm = document.forms[0];

	if( theForm.WLAuthType.value == "4" )
	{
		if( theForm.WPAPASS.value.length < 8 )
		{
			alert("英數字長度 8 到 63, 或十六進位長度 64!");
			return false;
		}
		else if( theForm.WPAPASS.value.indexOf(" ") >= 0 )
		{
			alert("網路安全金鑰中間不能有空白!");
			return false;
		}
		else
		{
			theForm.action=szURL;
			theForm.submit();
			return false;
		}
	}
	else
	{
		theForm.action=szURL;
		theForm.submit();
		return false;
	}
}

// out of Traditional Chinese
