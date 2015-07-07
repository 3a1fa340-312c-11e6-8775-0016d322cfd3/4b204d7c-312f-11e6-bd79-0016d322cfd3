Len_html = 7;
Len_tab = 13;

Len_text0 = 17;		// SYSTEM.HTM, PRINTJOB.HTM
Len_text1 = 5;		// PRINTER.HTM
Len_text2 = 12;		// NETWARE.HTM
Len_text3 = 8;		// TCPIP.HTM, Rendezvous
Len_text4 = 6;		// APPLE.HTM
Len_text5 = 8;		// SNMP.HTM
Len_text6 = 10;		// JOBLOG.HTM
Len_text7 = 4;		// SMB.HTM

htmArray = new Array(Len_html);
tabArray= new Array(Len_tab);
headArray= new Array(Len_html);

textArray0= new Array(Len_text0);
textArray1= new Array(Len_text1);
textArray2= new Array(Len_text2);
textArray3= new Array(Len_text3);
textArray4= new Array(Len_text4);
textArray5= new Array(Len_text5);
textArray6= new Array(Len_text6);
textArray7= new Array(Len_text7);

htmArray = ['system','printer','tcpip','netware','apple','snmp','smb'];
browserLangu = 'e';

function adjuestlanguage()
{
	var languages = navigator.browserLanguage;
	var userAgents = navigator.userAgent;
	var userLanguages = navigator.userLanguage;

	if(navigator.appName=="Netscape")
	{
		if(navigator.appVersion.indexOf("5.0")!=-1)
		{
			if(navigator.appVersion.indexOf("Safari")!=-1)
			{
				if(userAgents.indexOf("zh-tw")!=-1)
					browserLangu = 'z';
				else if(userAgents.indexOf("zh-cn")!=-1)
					browserLangu = 'c';
				else
					browserLangu = 'e';
			}
			else
			{
				if(userAgents.indexOf("zh-TW")!=-1)
					browserLangu = 'z';
				else if(userAgents.indexOf("zh-CN")!=-1)
					browserLangu = 'c';
				else
					browserLangu = 'e';
			}
		}
		else
			browserLangu = 'e';
	}
	else if(navigator.appName=="Konqueror")
	{
		if(navigator.appVersion.indexOf("5.0")!=-1)
		{
			if(userLanguages.indexOf("zh_TW")!=-1)
				browserLangu = 'z';
			else if(userLanguages.indexOf("zh_CN")!=-1)
				browserLangu = 'c';
			else
				browserLangu = 'e';
		}
		else
			browserLangu = 'e';
	}
	else
	{
		switch (languages){
			case "zh-tw":
				browserLangu = 'z';
				break;
			case "zh-cn":
				browserLangu = 'c';
				break;
		    default:
		    	browserLangu = 'e';
		}
	}
}

function makeAlphaNumeric(originalName)
{
	alphaNumName = "";
	pos=0;
	while (pos<originalName.length)
	{
		c=originalName.charAt(pos);
		if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
		{
		  alphaNumName = alphaNumName + c;
		}
		pos++;
	}
	return alphaNumName;
}

function showtab(iPoision)
{
	document.write(tabArray[iPoision]);
	return true;
}

function showhead(webname)
{
	for(var i=0;i<Len_html;i++){
		if(webname.toLowerCase() == htmArray[i])
			break;
	}	
	document.write(headArray[i]);
	return true;
}

function showtext(iPoision)
{
	document.write(textArray0[iPoision]);
	return true;
}
function showtext1(iPoision)
{
	document.write(textArray1[iPoision]);
	return true;
}

function showtext2(iPoision)
{
	document.write(textArray2[iPoision]);
	return true;
}

function showtext3(iPoision)
{
	document.write(textArray3[iPoision]);
	return true;
}

function showtext4(iPoision)
{
	document.write(textArray4[iPoision]);
	return true;
}

function showtext5(iPoision)
{
	document.write(textArray5[iPoision]);
	return true;
}

function showtext6(iPoision)
{
	document.write(textArray6[iPoision]);
	return true;
}

function showtext7(iPoision)
{
	document.write(textArray7[iPoision]);
	return true;
}

adjuestlanguage();
document.write('<SCRIPT language="JavaScript" src="status-'+browserLangu+'.js"></SCRIPT>');
