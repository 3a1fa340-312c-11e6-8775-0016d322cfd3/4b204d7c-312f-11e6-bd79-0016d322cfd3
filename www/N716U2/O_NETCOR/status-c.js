//vaiable
menuindex = 0;
tabindex = 0;
textindex = 0;

var iIndex = 0;

menuArray=['NP302','系统信息','系统设置','打印设置','网络设置','系统管理','售后服务'];
tabArray=['系统','打印机','服务器 IP','NetWare','AppleTalk','SNMP','SMB'];
//Language : Simplified Chinese

//system.htm 0
headArray[iIndex++] = "系统信息";
//system.htm
headArray[iIndex++] = "此页面显示打印服务器的基本信息。<BR>";
//printer.htm 0
headArray[iIndex++] = "打印机";
//printer.htm
headArray[iIndex++] = "显示连接到打印服务器的某台打印机的信息。<BR>注意: 如果您的打印机不支持 bi-directional 功能，某些信息将不被显示。";
//tcpip.htm 0
headArray[iIndex++] = "打印服务器 IP";
//tcpip.htm
headArray[iIndex++] = "显示打印服务器 IP 设置的相关信息。<BR>";
//netware.htm 0
headArray[iIndex++] = "NetWare";
//netware.htm
headArray[iIndex++] = "显示打印服务器 NetWare 设置的相关信息。<BR>";
//apple.htm 0
headArray[iIndex++] = "AppleTalk";
//apple.htm
headArray[iIndex++] = "显示打印服务器 AppleTalk 设置的相关信息。<BR>";
//snmp.htm 0
headArray[iIndex++] = "SNMP";
//snmp.htm
headArray[iIndex++] = "显示打印服务器 SNMP 设置的相关信息。<BR>";
//smb.htm 0
headArray[iIndex++] = "SMB";
//Smb.htm
headArray[iIndex++] = "显示打印机在网络上的共享设置的相关信息。<BR>";
iIndex = 0;



//system.htm
textArray0[iIndex++]="系统信息";
textArray0[iIndex++]="打印服务器名称 :";
textArray0[iIndex++]="服务器注释 :";
textArray0[iIndex++]="服务器位置 :";
textArray0[iIndex++]="系统运行时间 :";
textArray0[iIndex++]="软件版本 :";
textArray0[iIndex++]="MAC 地址 :";
textArray0[iIndex++]="E-mail 通知 :";
//textArray0[iIndex++]="关闭";
//textArray0[iIndex++]="启用";
//PRINTJOB.htm
textArray0[iIndex++]="打印任务";
textArray0[iIndex++]="任务";
textArray0[iIndex++]="用户";
textArray0[iIndex++]="任务执行时间";
textArray0[iIndex++]="协议";
textArray0[iIndex++]="端口";
textArray0[iIndex++]="状态";
textArray0[iIndex++]="打印字节数";
textArray0[iIndex++]="打印工作记录";
iIndex = 0;

//Printer.htm
textArray1[iIndex++]="打印机信息";
textArray1[iIndex++]="生产厂家 :";
textArray1[iIndex++]="型号 :";
textArray1[iIndex++]="支持语言 :";
textArray1[iIndex++]="当前状态 :";
//textArray1[iIndex++]="等待打印任务....";
//textArray1[iIndex++]="缺纸";
//textArray1[iIndex++]="脱机";
//textArray1[iIndex++]="正在打印";
iIndex = 0;

//NETWARE.htm
textArray2[iIndex++]="基本设置";
textArray2[iIndex++]="打印服务器名称 :";
textArray2[iIndex++]="轮询时间 :";
//textArray2[iIndex++]="秒";
textArray2[iIndex++]="NetWare NDS 设置";
textArray2[iIndex++]="使用 NDS 模式 :";
//textArray2[iIndex++]="关闭";
//textArray2[iIndex++]="启用";
textArray2[iIndex++]="NDS Tree 名称 :";
textArray2[iIndex++]="NDS Context 名称 :";
textArray2[iIndex++]="当前状态:";
//textArray2[iIndex++]="断开";
//textArray2[iIndex++]="已连接";
textArray2[iIndex++]="NetWare Bindery 设置";
textArray2[iIndex++]="使用 Bindery 模式 :";
//textArray2[iIndex++]="关闭";
//textArray2[iIndex++]="启用";
textArray2[iIndex++]="文件服务器名称 :";
textArray2[iIndex++]="当前状态 :";
//textArray2[iIndex++]="断开";
//textArray2[iIndex++]="已连接";
iIndex = 0;
//tcpip.htm
textArray3[iIndex++]="IP 设置";
textArray3[iIndex++]="DHCP/BOOTP 状态 :";
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
textArray4[iIndex++]="AppleTalk :";
textArray4[iIndex++]="打印机信息";
textArray4[iIndex++]="端口名称 :";
textArray4[iIndex++]="打印机类型 :";
textArray4[iIndex++]="数据格式 :";
iIndex = 0;
//SNMP.htm
textArray5[iIndex++]="SNMP 团体设置";
textArray5[iIndex++]="SNMP 团体 1 :";
//textArray5[iIndex++]="只读";
//textArray5[iIndex++]="读写";
textArray5[iIndex++]="SNMP 团体 2 :";
//textArray5[iIndex++]="只读";
//textArray5[iIndex++]="读写";
textArray5[iIndex++]="SNMP 警告设置";
textArray5[iIndex++]="发送 SNMP 警告 :";
//textArray5[iIndex++]="关闭";
//textArray5[iIndex++]="启用";
textArray5[iIndex++]="警告认证 :";
//textArray5[iIndex++]="关闭";
//textArray5[iIndex++]="启用";
textArray5[iIndex++]="警告地址 1 :";
textArray5[iIndex++]="警告地址 2 :";
iIndex = 0;

//JOBLOG.htm
// Translate                                  only "Refresh " is to be translated
textArray6[iIndex++]='<input type=button value=" 刷新 " onClick="window.location.reload()">';
textArray6[iIndex++]="打印任务";
textArray6[iIndex++]="任务";
textArray6[iIndex++]="用户";
textArray6[iIndex++]="任务执行时间";
textArray6[iIndex++]="协议";
textArray6[iIndex++]="端口";
textArray6[iIndex++]="状态";
textArray6[iIndex++]="打印字节数";
// Translate                                  only "Close " is to be translated
textArray6[iIndex++]='<input type=button value=" 关闭 " onClick="window.close()">';
iIndex = 0;

//SMB.htm
textArray7[iIndex++]="工作组";
textArray7[iIndex++]="工作组名称 :";
textArray7[iIndex++]="共享名称";
textArray7[iIndex++]="打印机 :";


// out of Simplified Chinese
