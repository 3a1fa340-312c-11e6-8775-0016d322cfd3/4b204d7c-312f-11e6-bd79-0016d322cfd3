Len_html = 8;
Len_tab = 2;

Len_text0 = 34;		// ONE.HTM
Len_text7 = 13;		// ERROR.HTM

htmArray = new Array(Len_html);

tabArray= new Array(Len_tab);

textArray0= new Array(Len_text0);
textArray7= new Array(Len_text7);

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
			if((navigator.appVersion.indexOf("Chrome")!=-1) || 
				(navigator.appVersion.indexOf("Maxthon")!=-1))
			{
				if((chromeLanguages.indexOf("zh-TW")!=-1) || 
					(chromeLanguages.indexOf("zh-HK")!=-1) || 
					(chromeLanguages.indexOf("zh-MO")!=-1) || 
					(chromeLanguages.indexOf("zh-HANT")!=-1))
					browserLangu = 'z';
//				else if((chromeLanguages.indexOf("zh-CN")!=-1) || 
//					(chromeLanguages.indexOf("zh-SG")!=-1) || 
//					(chromeLanguages.indexOf("zh-HANS")!=-1))
//					browserLangu = 'c';
				else
					browserLangu = 'e';
			}
			else if(navigator.appVersion.indexOf("Safari")!=-1)
			{
				if((userAgents.indexOf("zh-tw")!=-1) || 
					(userAgents.indexOf("zh-TW")!=-1) || 
					(chromeLanguages.indexOf("zh-TW")!=-1) || 
					(userAgents.indexOf("zh-hk")!=-1) || 
					(userAgents.indexOf("zh-HK")!=-1) || 
					(chromeLanguages.indexOf("zh-HK")!=-1) || 
					(userAgents.indexOf("zh-mo")!=-1) || 
					(userAgents.indexOf("zh-MO")!=-1) || 
					(chromeLanguages.indexOf("zh-MO")!=-1) || 
					(userAgents.indexOf("zh-hant")!=-1) || 
					(userAgents.indexOf("zh-HANT")!=-1) || 
					(chromeLanguages.indexOf("zh-HANT")!=-1))
					browserLangu = 'z';
//				else if((userAgents.indexOf("zh-cn")!=-1) || 
//					(userAgents.indexOf("zh-sg")!=-1) || 
//					(userAgents.indexOf("zh-hans")!=-1))
//					browserLangu = 'c';
				else
					browserLangu = 'e';
			}
			else if(navigator.appVersion.indexOf("Android")!=-1)
			{
				if((userAgents.indexOf("zh-tw")!=-1) || 
					(userAgents.indexOf("zh-hk")!=-1) || 
					(userAgents.indexOf("zh-mo")!=-1) || 
					(userAgents.indexOf("zh-hant")!=-1))
					browserLangu = 'z';
//				else if((userAgents.indexOf("zh-cn")!=-1) || 
//					(userAgents.indexOf("zh-sg")!=-1) || 
//					(userAgents.indexOf("zh-hans")!=-1))
//					browserLangu = 'c';
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
//				else if((userAgents.indexOf("zh-CN")!=-1) || 
//					(userAgents.indexOf("zh-SG")!=-1) || 
//					(userAgents.indexOf("zh-HANS")!=-1))
//					browserLangu = 'c';
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
//			else if((userLanguages.indexOf("zh_CN")!=-1) || 
//				(userLanguages.indexOf("zh_SG")!=-1) || 
//				(userLanguages.indexOf("zh_HANS")!=-1))
//				browserLangu = 'c';
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
//			case "zh-cn":
//			case "zh-sg":
//			case "zh-hans":
//				browserLangu = 'c';
//				break;
		    default:
		    	browserLangu = 'e';
		}
	}
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

function showtext(iPoision)
{
	document.write(textArray0[iPoision]);
	return true;
}

function showtext7(iPoision)
{
	document.write(textArray7[iPoision]);
	return true;
}

adjuestlanguage();
document.write('<SCRIPT language="JavaScript" src="setup-'+browserLangu+'.js"></SCRIPT>');
