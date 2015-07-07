Len_html = 8;
Len_tab = 12;

Len_text0 = 18;		// CSYSTEM.HTM
Len_text1 = 5;
Len_text2 = 12;
Len_text3 = 9;
Len_text4 = 24;		// CSNMP.HTM
Len_text5 = 7;
Len_text6 = 8;
Len_text7 = 13;		// ERROR.HTM
Len_text8 = 20;		// CNETWARE.HTM
Len_text9 = 7;		// CSMB.HTM
//Len_text10 = 21;	// CWLAN.HTM Basic Settings		// original:20, 10	With Wireless Country Code
Len_text10 = 11;	// CWLAN.HTM Basic Settings		// original:20, 10	Without Wireless Country Code
Len_text11 = 25;	// CWLAN.HTM Advanced Settings	// original:26
Len_text12 = 8;		// CWLAN.HTM Site Survey		// original:7

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
textArray8= new Array(Len_text8);
textArray9= new Array(Len_text9);
textArray10= new Array(Len_text10);
textArray11= new Array(Len_text11);
textArray12= new Array(Len_text12);

htmArray = ['csystem','cwlan','ctcpip','cnetware','capple','csnmp','csmb'];

browserLangu = 'e';

function adjuestlanguage()
{
	var languages = navigator.browserLanguage;
	var userAgents = navigator.userAgent;
	var userLanguages = navigator.userLanguage;
	var chromeLanguages = navigator.language;

	if(navigator.appName=="Netscape")
	{
		if(navigator.appVersion.indexOf("5.0")!=-1)
		{
			if(navigator.appVersion.indexOf("Chrome")!=-1)
			{
				if((chromeLanguages.indexOf("zh-TW")!=-1) || 
					(chromeLanguages.indexOf("zh-HK")!=-1) || 
					(chromeLanguages.indexOf("zh-MO")!=-1) || 
					(chromeLanguages.indexOf("zh-HANT")!=-1))
					browserLangu = 'z';
				else if((chromeLanguages.indexOf("zh-CN")!=-1) || 
					(chromeLanguages.indexOf("zh-SG")!=-1) || 
					(chromeLanguages.indexOf("zh-HANS")!=-1))
					browserLangu = 'c';
				else
					browserLangu = 'e';
			}
			else if(navigator.appVersion.indexOf("Safari")!=-1)
			{
				if((userAgents.indexOf("zh-tw")!=-1) || 
					(userAgents.indexOf("zh-TW")!=-1) || 
					(userAgents.indexOf("zh-hk")!=-1) || 
					(userAgents.indexOf("zh-HK")!=-1) || 
					(userAgents.indexOf("zh-mo")!=-1) || 
					(userAgents.indexOf("zh-MO")!=-1) || 
					(userAgents.indexOf("zh-hant")!=-1) || 
					(userAgents.indexOf("zh-HANT")!=-1))
					browserLangu = 'z';
				else if((userAgents.indexOf("zh-cn")!=-1) || 
					(userAgents.indexOf("zh-CN")!=-1) || 
					(userAgents.indexOf("zh-sg")!=-1) || 
					(userAgents.indexOf("zh-SG")!=-1) || 
					(userAgents.indexOf("zh-hans")!=-1) || 
					(userAgents.indexOf("zh-HANS")!=-1))
					browserLangu = 'c';
				else
					browserLangu = 'e';
			}
			else
			{
				if((userAgents.indexOf("zh-TW")!=-1) || 
					(chromeLanguages.indexOf("zh-TW")!=-1) || 
					(userAgents.indexOf("zh-HK")!=-1) || 
					(chromeLanguages.indexOf("zh-HK")!=-1) || 
					(userAgents.indexOf("zh-MO")!=-1) || 
					(chromeLanguages.indexOf("zh-MO")!=-1) || 
					(userAgents.indexOf("zh-HANT")!=-1) || 
					(chromeLanguages.indexOf("zh-HANT")!=-1))
					browserLangu = 'z';
				else if((userAgents.indexOf("zh-CN")!=-1) || 
					(chromeLanguages.indexOf("zh-CN")!=-1) || 
					(userAgents.indexOf("zh-SG")!=-1) || 
					(chromeLanguages.indexOf("zh-SG")!=-1) || 
					(userAgents.indexOf("zh-HANS")!=-1) || 
					(chromeLanguages.indexOf("zh-HANS")!=-1))
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
			if((userLanguages.indexOf("zh_TW")!=-1) || 
				(userLanguages.indexOf("zh_HK")!=-1) || 
				(userLanguages.indexOf("zh_MO")!=-1) || 
				(userLanguages.indexOf("zh_HANT")!=-1))
				browserLangu = 'z';
			else if((userLanguages.indexOf("zh_CN")!=-1) || 
				(userLanguages.indexOf("zh_SG")!=-1) || 
				(userLanguages.indexOf("zh_HANS")!=-1))
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
			case "zh-hk":
			case "zh-mo":
			case "zh-hant":
				browserLangu = 'z';
				break;
			case "zh-cn":
			case "zh-sg":
			case "zh-hans":
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

function SaveSetting(szURL)
{
	document.forms[0].action=szURL;
	document.forms[0].submit();
	return false;
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

function showtext8(iPoision)
{
	document.write(textArray8[iPoision]);
	return true;
}

function showtext9(iPoision)
{
	document.write(textArray9[iPoision]);
	return true;
}

function showtext10(iPoision)
{
	document.write(textArray10[iPoision]);
	return true;
}

function showtext11(iPoision)
{
	document.write(textArray11[iPoision]);
	return true;
}

function showtext12(iPoision)
{
	document.write(textArray12[iPoision]);
	return true;
}
adjuestlanguage();
document.write('<SCRIPT language="JavaScript" src="setup-'+browserLangu+'.js"></SCRIPT>');
