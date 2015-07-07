

//vaiable
menuindex = 0;
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Simplified Chinese
menuArray=['NP302','系统信息','系统设置','打印设置','网络设置','系统管理','售后服务'];
tabArray=['系统重启','系统升级','恢复缺省','用户密码设置'];

//reset.htm 0
headArray[iIndex++] ="系统重启";
//reset.htm
headArray[iIndex++] ="重新启动打印服务器。";
//upgrade.htm 0
headArray[iIndex++] ="系统升级";
//upgrade.htm
headArray[iIndex++] ="您可以升级打印服务器的版本 (请选择正确的升级文件, 如果不知道应该选择何种升级文件, 请联系当地的服务商或技术支持)。";
//default.htm 0
headArray[iIndex++] ="恢复缺省";
//default.htm
headArray[iIndex++] ="将打印服务器恢复到出厂设置，所有参数将丢失。";
//password.htm 0
headArray[iIndex++] ="用户密码设置";
//password.htm
headArray[iIndex++] ="在这里你可以修改密码。";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "确定将打印服务器恢复到出厂设置吗? (警告! 当前所有参数将丢失!)";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;确定&nbsp;&nbsp;" class=input_more_back onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="版本升级";
textArray2[iIndex++]="选择升级文件 :";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="保存生效" class=input_more_back onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>重启打印服务器。</B></FONT>";
textArray3[iIndex++]="确定保存设置并重启打印服务器吗?";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;确定&nbsp;&nbsp;" class=input_more_back onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

//password.htm
textArray3a[iIndex++]="用户密码设置";
textArray3a[iIndex++]="新密码:";
textArray3a[iIndex++]="确认新密码:";
// Translate                               Only OK is to be translated
textArray3a[iIndex++]='<input type=button value="保存生效" class=input_more_back onClick="return CheckPwd(';
// Begin don't translate
textArray3a[iIndex++]="'RESTART.HTM');";
textArray3a[iIndex++]='">';
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
textArray6[iIndex++] = "回复出厂默认值后，此打印服务器自动重新启动。<BR><BR>重新启动后, 打印服务器会带您回到主页.";
iIndex = 0;

	//support.htm
textArray7[iIndex++] = "售后服务";
textArray7[iIndex++] = "Netcore中国公司于2000年7月正式宣布成立，现已在中国地区设立了一个营销中心，一个生产基地和两个研发中心，并在国内主要中心城市建立了10个销售办事处，辐射全国网络市场。Netcore中国公司旨在为国内用户提供经济、高效、实用的全面网络解决方案，产品包括网卡、集线器、交换机、路由器、调制解调器等多个系列。并致力于三层到七层交换、全光网等前端科技的研发与应用。";
textArray7[iIndex++] = "“专注用户需求、推动网络应用”，Netcore中国公司将与广大用户一起迎接信息时代的挑战！";
textArray7[iIndex++] = "欲获得更多有关磊科网络技术服务支持，请致电磊科免长途技术支持热线：400-810-1616";
textArray7[iIndex++] = "磊科官方网站";
textArray7[iIndex++] = "适合全国用户访问";
textArray7[iIndex++] = "磊科官方网站(www.netcoretec.com)";
textArray7[iIndex++] = "在这里您可以获得最新的产品信息及软件升级版本；客户服务技术支持中心提供最详尽的技术支持，技术支持人员一对一地为您解答在我们产品使用过程中遇到的问题。";
textArray7[iIndex++] = "磊科北方网站";
textArray7[iIndex++] = "适合北方网通用户访问";
textArray7[iIndex++] = "磊科北方网站(www.netcore.com.cn)";
textArray7[iIndex++] = "北方网站适合北方网通用户访问，及时发布最新软件升级版本，为您解决遇到的各种问题。";
iIndex = 0;
// out of Simplified Chinese
