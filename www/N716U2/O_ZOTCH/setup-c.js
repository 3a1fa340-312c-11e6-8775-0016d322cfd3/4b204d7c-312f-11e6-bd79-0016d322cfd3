
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Simplified Chinese
tabArray=['PU201 高速 USB 打印服务器','状态','设置','其它','重启','系统','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>此屏幕允许您在一般的操作系统下设置打印服务器。<br>";
//cprinter.htm
//headArray[iIndex++] = "<BR>此屏幕允许您启动或者关闭打印服务器每个端口的双向打印功能。";
//ctcpip.htm
headArray[iIndex++] = "<BR>此屏幕允许您更改打印服务器的 TCP/IP 设置。";
//cnetware.htm
headArray[iIndex++] = "<BR>此屏幕允许您更改打印服务器的 NetWare 设置。";
//capple.htm 
headArray[iIndex++] ="<BR>此屏幕允许您更改打印服务器的 AppleTalk 设置。<br>";
//csnmp.htm
headArray[iIndex++] ="<BR>此屏幕允许您更改打印服务器的 SNMP 设置。";
//csmp.htm
headArray[iIndex++] ="<BR>此屏幕允许您更改打印服务器在微软网上邻居的共享设置。";
iIndex = 0;

//csystem.htm
textArray0[iIndex++]="电邮通知设置";
textArray0[iIndex++]="电邮通知:";
textArray0[iIndex++]="SMTP 服务器 IP 地址:";
textArray0[iIndex++]="管理员电邮地址:";
textArray0[iIndex++]="系统设置";
textArray0[iIndex++]="设备名称 :";
textArray0[iIndex++]="系统注释 :";
textArray0[iIndex++]="位置 :";
textArray0[iIndex++]="管理员密码";
textArray0[iIndex++]="密码 :";
textArray0[iIndex++]="确认密码 :";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="保存并重启" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
//CPRINTER.htm
textArray1[iIndex++]="双向打印功能设置";
textArray1[iIndex++]="打印服务器第一端口 :";
textArray1[iIndex++]='<input type="button" value="保存并重启" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
//CTCPIP.htm
textArray2[iIndex++]="TCP/IP 设置";
textArray2[iIndex++]="自动获得 TCP/IP 设置 (使用 DHCP/BOOTP)";
textArray2[iIndex++]="使用以下 TCP/IP 设置";
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
textArray2[iIndex++]='<input type="button" value="保存并重启" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="AppleTalk 设置";
textArray3[iIndex++]="AppleTalk Zone 名称 :";
textArray3[iIndex++]="端口名称 :";
textArray3[iIndex++]="打印机";
textArray3[iIndex++]="打印机类型 :";
textArray3[iIndex++]="数据格式 :";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="保存并重启" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
//CSNMP.htm
textArray4[iIndex++]="SNMP Community 设置";
textArray4[iIndex++]="允许使用 HP WebJetAdmin :";
textArray4[iIndex++]="Community 1 名称 :";
textArray4[iIndex++]="权限 :";
textArray4[iIndex++]="Community 2 名称 :";
textArray4[iIndex++]="权限 :";
textArray4[iIndex++]="SNMP Trap 设置";
textArray4[iIndex++]="发送 SNMP Traps :";
textArray4[iIndex++]="使用授权的 Traps :";
textArray4[iIndex++]="发送 Traps 到第一个 IP 地址 :";
textArray4[iIndex++]="发送 Traps 到第二个 IP 地址 :";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="保存并重启" onClick="return SaveSetting(';
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
textArray7[iIndex++]="后退";
iIndex = 0;

// CNETWARE.htm
textArray8[iIndex++]="基本设置";
textArray8[iIndex++]="打印服务器名称 :";
textArray8[iIndex++]="轮询时间 :";
textArray8[iIndex++]="&nbsp;秒 (3-29)";
textArray8[iIndex++]="登录密码 :";
textArray8[iIndex++]="NetWare NDS 设置";
textArray8[iIndex++]="使用 NDS 模式 :";
textArray8[iIndex++]="NDS Tree 名称 :";
textArray8[iIndex++]="NDS Context 名称 :";
textArray8[iIndex++]="NetWare Bindery 设置";
textArray8[iIndex++]="使用 Bindery 模式 :";
textArray8[iIndex++]="文件服务器名称 :";
textArray8[iIndex++]='<input type="button" value="保存并重启" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.htm
textArray9[iIndex++]="工作组";
textArray9[iIndex++]="名称 :";
textArray9[iIndex++]="共享打印机名称";
textArray9[iIndex++]="共享名 :";
textArray9[iIndex++]='<input type="button" value="保存并重启" onClick="return CheckSMB(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;

// functions
// CSYSTEM.HTM
//function CheckPwd(szURL)
//{
// 	if(document.forms[0].SetupPWD.value != document.forms[0].ConfirmPWD.value && document.forms[0].SetupPWD.value != "ZO__I-SetupPassword" )
//	{
//		alert("管理员密码和确认密码不匹配 !");
//		return false;
//	}
//	document.forms[0].action=szURL;
//	document.forms[0].submit();
//	return false;
//}

// CPRINTER.HTM

// CTCPTP.HTM

// CNETWARE.HTM

// CAPPLE.HTM

// CSNMP.HTM

// CSMB.HTM


// out of Simplified Chinese
