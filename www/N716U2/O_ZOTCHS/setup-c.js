
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Simplified Chinese
tabArray=['PU211S USB 打印服务器',''];

//csystem.htm
textArray0[iIndex++]='<input type=button value="&nbsp;&nbsp;重新启动&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM'";
textArray0[iIndex++]='">';
textArray0[iIndex++]="打印工作记录";

textArray0[iIndex++]="系统信息";
textArray0[iIndex++]="设备名称 :";
textArray0[iIndex++]="系统启动时间 :";
textArray0[iIndex++]="固件版本 :";
textArray0[iIndex++]="MAC 地址 :";

textArray0[iIndex++]="TCP/IP 设置";
textArray0[iIndex++]="自动获得 TCP/IP 设置 (使用 DHCP/BOOTP)";
textArray0[iIndex++]="使用以下 TCP/IP 设置";
textArray0[iIndex++]="IP 地址 :";
textArray0[iIndex++]="子网掩码 :";
textArray0[iIndex++]="默认网关 :";

textArray0[iIndex++]="管理员密码";
textArray0[iIndex++]="用户名 :";
textArray0[iIndex++]="密码 :";
textArray0[iIndex++]="确认密码 :";

textArray0[iIndex++]="打印机信息";
textArray0[iIndex++]="生产商 :";
textArray0[iIndex++]="型号 :";
textArray0[iIndex++]="支持的语言 :";
textArray0[iIndex++]="当前状态 :";
textArray0[iIndex++]="等待打印任务....";
textArray0[iIndex++]="缺纸";
textArray0[iIndex++]="脱机";
textArray0[iIndex++]="正在打印";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="保存并重启" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';

textArray0[iIndex++]="升级固件";
textArray0[iIndex++]="选择文件:";
// Begin don't translate
textArray0[iIndex++]='<input type=button value="升级固件" onClick="return WebUpgrade()">';

iIndex = 0;
// End don't translate

// ERROR.htm
textArray7[iIndex++]="ERROR";
textArray7[iIndex++]="无效 IP 地址";
textArray7[iIndex++]="无效子网掩码";
textArray7[iIndex++]="无效网关地址";
textArray7[iIndex++]="Polling Time 值无效";
textArray7[iIndex++]="无效打印服务器名";
textArray7[iIndex++]="无效文件服务器名";
textArray7[iIndex++]="未找到 DHCP/BOOTP 服务器";
textArray7[iIndex++]="无效 SNMP Trap IP 地址";
textArray7[iIndex++]="密码不匹配";
textArray7[iIndex++]="软件失效或升级失败";
textArray7[iIndex++]="";
textArray7[iIndex++]="后退";
iIndex = 0;

// functions
// CSYSTEM.HTM
function CheckPwd(szURL)
{
 	if(document.forms[0].SetupPWD.value != document.forms[0].ConfirmPWD.value && document.forms[0].SetupPWD.value != "ZO__I-SetupPassword" )
	{
		alert("管理员密码和确认密码不匹配 !");
		return false;
	}
	document.forms[0].action=szURL;
	document.forms[0].submit();
	return false;
}


// out of Simplified Chinese