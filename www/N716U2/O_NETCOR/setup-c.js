
//vaiable
menuindex = 0;
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Simplified Chinese
menuArray=['NP302','系统信息','系统设置','打印设置','网络设置','系统管理','售后服务'];
tabArray=['打印机','服务器 IP','NetWare','AppleTalk','SNMP','SMB'];

//csystem.htm 0
headArray[iIndex++] = "打印机";
//csystem.htm
headArray[iIndex++] = "该配置项允许你配置服务器系统的相关参数。";
//ctcpip.htm 0
headArray[iIndex++] = "服务器 IP 设置";
//ctcpip.htm
headArray[iIndex++] = "该配置项允许你配置服务器 IP 的相关参数。";
//cnetware.htm 0
headArray[iIndex++] = "NetWare 设置";
//cnetware.htm
headArray[iIndex++] = "该配置项允许你配置打印服务器的 NetWare 模块。";
//capple.htm 0
headArray[iIndex++] = "ApplaTalk 设置";
//capple.htm 
headArray[iIndex++] ="该配置项允许你为打印服务器的 AppleTalk 模块配置相关参数。";
//csnmp.htm 0
headArray[iIndex++] = "SNMP 设置";
//csnmp.htm
headArray[iIndex++] ="该配置项允许你配置 SNMP 的相关参数。";
//csmb.htm 0
headArray[iIndex++] = "SMB 设置";
//csmb.htm
headArray[iIndex++] ="配置打印机显示在网络上的共享信息。";
iIndex = 0;

//csystem.htm
textArray0[iIndex++]="邮件通知设置";
textArray0[iIndex++]="邮件通知:";
//textArray0[iIndex++]="关闭";
//textArray0[iIndex++]="启用";
textArray0[iIndex++]="SMTP 服务器 IP 地址:";
textArray0[iIndex++]="管理员 E-mail:";
textArray0[iIndex++]="系统设置";
textArray0[iIndex++]="打印服务器名称 :";
textArray0[iIndex++]="服务器注释 :";
textArray0[iIndex++]="服务器位置 :";
//textArray0[iIndex++]="管理员密码";
//textArray0[iIndex++]="用户名 :";
//textArray0[iIndex++]="admin";
//textArray0[iIndex++]="密码 :";
//textArray0[iIndex++]="确认密码 :";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="保存生效" class=input_more_back onClick="return SaveSetting(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
//CPRINTER.htm
textArray1[iIndex++]="双向打印功能设置";
textArray1[iIndex++]="打印服务器第一端口 :";
textArray1[iIndex++]='<input type="button" value="保存生效" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
//CTCPIP.htm
textArray2[iIndex++]="打印服务器 IP 设置";
textArray2[iIndex++]="使用 DHCP/BOOTP 自动获取 IP 地址";
textArray2[iIndex++]="手动配置静态 IP 地址";
textArray2[iIndex++]="IP 地址 :";
textArray2[iIndex++]="子网掩码 :";
textArray2[iIndex++]="默认网关 :";
//crandvoo.htm
textArray2[iIndex++]="Rendezvous 设置";
textArray2[iIndex++]="Rendezvous 设置 :";
//textArray2[iIndex++]="Disable";
//textArray2[iIndex++]="Enable";
textArray2[iIndex++]="服务名称 :";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="保存生效" class=input_more_back onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="AppleTalk 设置";
textArray3[iIndex++]="AppleTalk 域 :";
textArray3[iIndex++]="端口名称 :";
textArray3[iIndex++]="打印机配置";
textArray3[iIndex++]="类型 :";
textArray3[iIndex++]="协议类型 :";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="保存生效" class=input_more_back onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
//CSNMP.htm
textArray4[iIndex++]="SNMP 团体设置";
textArray4[iIndex++]="支持 HP WebJetAdmin :";
//textArray4[iIndex++]="关闭";
//textArray4[iIndex++]="启用";
textArray4[iIndex++]="SNMP 团体名称 1 :";
textArray4[iIndex++]="权限 :";
//textArray4[iIndex++]="只读";
//textArray4[iIndex++]="读写";
textArray4[iIndex++]="SNMP 团体名称 2 :";
textArray4[iIndex++]="权限 :";
//textArray4[iIndex++]="只读";
//textArray4[iIndex++]="读写";
textArray4[iIndex++]="SNMP 警告设置";
textArray4[iIndex++]="发送 SNMP 警告 :";
//textArray4[iIndex++]="关闭";
//textArray4[iIndex++]="启用";
textArray4[iIndex++]="认证警告 :";
//textArray4[iIndex++]="关闭";
//textArray4[iIndex++]="启用";
textArray4[iIndex++]="警告地址 1 :";
textArray4[iIndex++]="警告地址 2 :";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="保存生效" class=input_more_back onClick="return SaveSetting(';
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
textArray5[iIndex++]='<INPUT TYPE=button VALUE=" 关闭 " onClick="window.close()">';
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
textArray7[iIndex++]="ERROR";
//textArray7[iIndex++]="无效 IP 地址";
//textArray7[iIndex++]="无效子网掩码";
//textArray7[iIndex++]="无效网关地址";
//textArray7[iIndex++]="Polling Time 值无效";
//textArray7[iIndex++]="无效打印服务器名";
//textArray7[iIndex++]="无效文件服务器名";
//textArray7[iIndex++]="未找到 DHCP/BOOTP 服务器";
//textArray7[iIndex++]="无效 SNMP Trap IP 地址";
//textArray7[iIndex++]="密码不匹配";
//textArray7[iIndex++]="软件失效或升级失败";
//textArray7[iIndex++]="";
textArray7[iIndex++]="后退";
iIndex = 0;

// CNETWARE.htm
textArray8[iIndex++]="基本配置";
textArray8[iIndex++]="打印服务器名称 :";
textArray8[iIndex++]="轮询时间 :";
textArray8[iIndex++]="&nbsp;秒 (最小: 3 秒, 最大: 29 秒)";
textArray8[iIndex++]="登录密码 :";
textArray8[iIndex++]="NetWare NDS 配置";
textArray8[iIndex++]="NDS 模式 :";
//textArray8[iIndex++]="关闭";
//textArray8[iIndex++]="启用";
textArray8[iIndex++]="NDS 树 名称 :";
textArray8[iIndex++]="NDS 描述 :";
textArray8[iIndex++]="NetWare Bindery 配置";
textArray8[iIndex++]="Bindery 模式 :";
//textArray8[iIndex++]="关闭";
//textArray8[iIndex++]="启用";
textArray8[iIndex++]="文件服务器名称 :";
//textArray8[iIndex++]="没有找到文件服务器 !";
textArray8[iIndex++]='<input type="button" value="保存生效" class=input_more_back onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.htm
textArray9[iIndex++]="工作组";
textArray9[iIndex++]="工作组名称 :";
textArray9[iIndex++]="共享名称";
textArray9[iIndex++]="打印机 :";
textArray9[iIndex++]='<input type="button" value="保存生效" class=input_more_back onClick="return CheckSMB(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;

// functions
// CSYSTEM.HTM

// CPRINTER.HTM

// CTCPTP.HTM

// CNETWARE.HTM

// CAPPLE.HTM

// CSNMP.HTM

// CSMB.HTM
function CheckSMB(szURL)
{
	if(document.forms[0].SMBWorkGroup.value == '')
	{
		alert("错误! 工作组名称不能空白!");
		return false;
	}
	else
	{
		if(document.forms[0].SMBPrint1.value == '')
		{		
			alert("错误! 共享打印机名称不能空白!");
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

// out of Simplified Chinese
