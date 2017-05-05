
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : English
tabArray=['PU211S USB 印表伺服器',''];

//csystem.htm
textArray0[iIndex++]='<input type=button value="&nbsp;&nbsp;重新啟動&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM'";
textArray0[iIndex++]='">';
textArray0[iIndex++]="觀看列印紀錄";

textArray0[iIndex++]="系統資訊";
textArray0[iIndex++]="裝置名稱 :";
textArray0[iIndex++]="開機時間 :";
textArray0[iIndex++]="韌體版本 :";
textArray0[iIndex++]="網路卡位址 :";

textArray0[iIndex++]="TCP/IP 設定";
textArray0[iIndex++]="自動取得 IP 位址 (使用 DHCP/BOOTP)";
textArray0[iIndex++]="指定 IP 位址";
textArray0[iIndex++]="IP 位址 :";
textArray0[iIndex++]="子網路遮罩 :";
textArray0[iIndex++]="預設閘道器 :";

textArray0[iIndex++]="系統管理者密碼";
textArray0[iIndex++]="帳號 :";
textArray0[iIndex++]="密碼 :";
textArray0[iIndex++]="密碼確認 :";

textArray0[iIndex++]="印表機";
textArray0[iIndex++]="印表機廠牌 :";
textArray0[iIndex++]="印表機型號 :";
textArray0[iIndex++]="支援的列印語言 :";
textArray0[iIndex++]="目前狀態 :";
textArray0[iIndex++]="待機中";
textArray0[iIndex++]="缺紙";
textArray0[iIndex++]="未連接或印表機離線";
textArray0[iIndex++]="列印中";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="儲存並重新啟動" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';

textArray0[iIndex++]="韌體升級";
textArray0[iIndex++]="選擇檔案:";
// Begin don't translate
textArray0[iIndex++]='<input type=button value="韌體升級" onClick="return WebUpgrade()">';

iIndex = 0;
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

// out of Chinese
