//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

//tabArray=['<IMG SRC=images/LOGO.gif>','状态','设置','其它','重启','系统','打印机','无线','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
tabArray=['无线/有线 双模打印服务器','状态','设置','其它','重启','系统','打印机','无线','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
//Language : Simplified Chinese

//system.htm
headArray[iIndex++] = "<BR>此屏幕显示打印服务器的基本信息。<BR>";
//printer.htm
headArray[iIndex++] = "<BR>此屏幕显示连接到打印服务器的每台打印机的详细信息。<BR>注意: 如果您的打印机不支持双向打印功能，一些信息将不会正确地显示出来。";
//wlan.htm
headArray[iIndex++] = "<BR>此屏幕显示打印服务器当前无线设置。";
//tcpip.htm
headArray[iIndex++] = "<BR>此屏幕显示打印服务器当前 TCP/IP 设置。<BR>";
//netware.htm
headArray[iIndex++] = "<BR>此屏幕显示打印服务器当前 NetWare 设置。<BR>";
//apple.htm
headArray[iIndex++] = "<BR>此屏幕显示打印服务器当前 AppleTalk 设置。<BR>";
//snmp.htm
headArray[iIndex++] = "<BR>此屏幕显示打印服务器当前 SNMP 设置。<BR>";
//Smb.htm
headArray[iIndex++] = "<BR>此屏幕显示打印服务器在微软网上邻居的共享设置。<BR>";
iIndex = 0;



//system.htm
textArray0[iIndex++]="系统信息";
textArray0[iIndex++]="设备名称 :";
textArray0[iIndex++]="系统注释 :";
textArray0[iIndex++]="位置 :";
textArray0[iIndex++]="系统启动时间 :";
textArray0[iIndex++]="固件版本 :";
textArray0[iIndex++]="MAC 地址 :";
textArray0[iIndex++]="电邮通知 :";
textArray0[iIndex++]="关闭";
textArray0[iIndex++]="启用";
//PRINTJOB.htm
textArray0[iIndex++]="打印工作";
textArray0[iIndex++]="工作编号";
textArray0[iIndex++]="所有者";
textArray0[iIndex++]="花费时间";
textArray0[iIndex++]="协议";
textArray0[iIndex++]="端口";
textArray0[iIndex++]="状态";
textArray0[iIndex++]="大小(Bytes)";
textArray0[iIndex++]="打印工作记录";
iIndex = 0;

//Printer.htm
textArray1[iIndex++]="打印机信息";
textArray1[iIndex++]="生产商 :";
textArray1[iIndex++]="型号 :";
textArray1[iIndex++]="支持的语言 :";
textArray1[iIndex++]="当前状态 :";
textArray1[iIndex++]="等待打印任务....";
textArray1[iIndex++]="缺纸";
textArray1[iIndex++]="脱机";
textArray1[iIndex++]="正在打印";
iIndex = 0;

//NETWARE.htm
textArray2[iIndex++]="基本设置";
textArray2[iIndex++]="打印服务器名称 :";
textArray2[iIndex++]="轮询时间 :";
textArray2[iIndex++]="秒";
textArray2[iIndex++]="NetWare NDS 设置";
textArray2[iIndex++]="使用 NDS 模式 :";
textArray2[iIndex++]="关闭";
textArray2[iIndex++]="启用";
textArray2[iIndex++]="NDS Tree 名称 :";
textArray2[iIndex++]="NDS Context 名称 :";
textArray2[iIndex++]="当前状态:";
textArray2[iIndex++]="断开";
textArray2[iIndex++]="已连接";
textArray2[iIndex++]="NetWare Bindery 设置";
textArray2[iIndex++]="使用 Bindery 模式 :";
textArray2[iIndex++]="关闭";
textArray2[iIndex++]="启用";
textArray2[iIndex++]="文件服务器名称 :";
textArray2[iIndex++]="当前状态 :";
textArray2[iIndex++]="断开";
textArray2[iIndex++]="已连接";
iIndex = 0;
//tcpip.htm
textArray3[iIndex++]="TCP/IP 设置";
textArray3[iIndex++]="使用 DHCP/BOOTP :";
textArray3[iIndex++]="IP 地址 :";
textArray3[iIndex++]="子网掩码 :";
textArray3[iIndex++]="网关 :";
//randvoo.htm
textArray3[iIndex++]="Rendezvous Settings";
textArray3[iIndex++]="Rendezvous Settings :";
//textArray3[iIndex++]="Disabled";
//textArray3[iIndex++]="Enabled";
textArray3[iIndex++]="Service Name :";
iIndex = 0;
//APPLE.htm
textArray4[iIndex++]="AppleTalk 设置";
textArray4[iIndex++]="AppleTalk Zone 名称 :";
textArray4[iIndex++]="打印机";
textArray4[iIndex++]="端口名称 :";
textArray4[iIndex++]="打印机类型 :";
textArray4[iIndex++]="数据格式 :";
iIndex = 0;
//SNMP.htm
textArray5[iIndex++]="SNMP Community 设置";
textArray5[iIndex++]="Community 1 名称 :";
textArray5[iIndex++]="只读";
textArray5[iIndex++]="读写";
textArray5[iIndex++]="Community 2 名称 :";
textArray5[iIndex++]="只读";
textArray5[iIndex++]="读写";
textArray5[iIndex++]="SNMP Trap 设置";
textArray5[iIndex++]="发送 SNMP Traps :";
textArray5[iIndex++]="关闭";
textArray5[iIndex++]="启用";
textArray5[iIndex++]="使用授权的 Traps :";
textArray5[iIndex++]="关闭";
textArray5[iIndex++]="启用";
textArray5[iIndex++]="发送 Traps 到第一个 IP 地址 :";
textArray5[iIndex++]="发送 Traps 到第二个 IP 地址 :";
iIndex = 0;

//JOBLOG.htm
// Translate                                  only "Refresh " is to be translated
textArray6[iIndex++]='<input type=button value=" 刷新 " onClick="window.location.reload()">';
textArray6[iIndex++]="打印工作";
textArray6[iIndex++]="工作编号";
textArray6[iIndex++]="所有者";
textArray6[iIndex++]="花费时间";
textArray6[iIndex++]="协议";
textArray6[iIndex++]="端口";
textArray6[iIndex++]="状态";
textArray6[iIndex++]="大小(Bytes)";
// Translate                                  only "Close " is to be translated
textArray6[iIndex++]='<input type=button value=" 关闭 " onClick="window.close()">';
iIndex = 0;

//SMB.htm
textArray7[iIndex++]="工作组";
textArray7[iIndex++]="名称 :";
textArray7[iIndex++]="共享打印机名称";
textArray7[iIndex++]="共享名 :";
iIndex = 0;

//WLAN.htm
textArray8[iIndex++]="<BR><BR><FONT COLOR=RED>请注意 ! 这个打印服务器目前在 Diagnostic 模式.</FONT><BR>";
textArray8[iIndex++]="无线状态信息";
textArray8[iIndex++]="无线类型 :";
textArray8[iIndex++]="网络类型 :";
textArray8[iIndex++]="AP 的 MAC 地址:";
textArray8[iIndex++]="SSID:";
textArray8[iIndex++]="频段 :";
textArray8[iIndex++]="无线模式 :";
textArray8[iIndex++]="传输速率 :";
textArray8[iIndex++]="Rx 信号密度 :";
textArray8[iIndex++]="连结品质 :";
textArray8[iIndex++]="安全类型 :";
textArray8[iIndex++]="认证方式 :";
iIndex = 0;


// out of Simplified Chinese
