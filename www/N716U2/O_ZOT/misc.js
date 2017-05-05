Len_html = 1;
Len_tab = 8;

Len_text1 = 6;		// DEFAULT.HTM	original: 5
Len_text2 = 3;		// UPGRADE.HTM
Len_text3 = 4;		// RESET.HTM
Len_text4 = 2;		// DRESTART.HTM
Len_text5 = 2;		// URESTART.HTM
Len_text6 = 2;		// RESTART.HTM

htmArray = new Array(Len_html);

tabArray= new Array(Len_tab);
headArray= new Array(Len_html);
textArray1= new Array(Len_text1);
textArray2= new Array(Len_text2);
textArray3= new Array(Len_text3);
textArray4= new Array(Len_text4);
textArray5= new Array(Len_text5);
textArray6= new Array(Len_text6);

htmArray = ['upgrade.htm'];

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
				
//				if(userAgents.indexOf("fr")!=-1)
//					browserLangu = 'f';
//				else if(userAgents.indexOf("de-de")!=-1)
//					browserLangu = 'd';
//				else if(userAgents.indexOf("it-it")!=-1)
//					browserLangu = 'i';
//				else if(userAgents.indexOf("es")!=-1)
//					browserLangu = 's';
//				else if(userAgents.indexOf("ja-jp")!=-1)
//					browserLangu = 'j';
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
				else
					browserLangu = 'e';
			}
			else
			{
//				if(userAgents.indexOf("fr-FR")!=-1)
//					browserLangu = 'f';		
//				else if(userAgents.indexOf("de-DE")!=-1)
//					browserLangu = 'd';
//				else if(userAgents.indexOf("de-AT")!=-1)
//					browserLangu = 'd';
//				else if(userAgents.indexOf("it-IT")!=-1)
//					browserLangu = 'i';
//				else if(userAgents.indexOf("es-ES")!=-1)
//					browserLangu = 's';
//				else if(userAgents.indexOf("ja-JP")!=-1)
//					browserLangu = 'j';
				if((userAgents.indexOf("zh-TW")!=-1) || 
					(chromeLanguages.indexOf("zh-TW")!=-1) || 
					(userAgents.indexOf("zh-HK")!=-1) || 
					(chromeLanguages.indexOf("zh-HK")!=-1) || 
					(userAgents.indexOf("zh-MO")!=-1) || 
					(chromeLanguages.indexOf("zh-MO")!=-1) || 
					(userAgents.indexOf("zh-HANT")!=-1) || 
					(chromeLanguages.indexOf("zh-HANT")!=-1))
					browserLangu = 'z';
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
//			if(userLanguages.indexOf("fr")!=-1)
//				browserLangu = 'f';
//			else if(userLanguages.indexOf("de")!=-1)
//				browserLangu = 'd';
//			else if(userLanguages.indexOf("it")!=-1)
//				browserLangu = 'i';
//			else if(userLanguages.indexOf("es")!=-1)
//				browserLangu = 's';
//			else if(userLanguages.indexOf("ja")!=-1)
//				browserLangu = 'j';
			if((userLanguages.indexOf("zh_TW")!=-1) || 
				(userLanguages.indexOf("zh_HK")!=-1) || 
				(userLanguages.indexOf("zh_MO")!=-1) || 
				(userLanguages.indexOf("zh_HANT")!=-1))
				browserLangu = 'z';
			else
				browserLangu = 'e';
		}
		else
			browserLangu = 'e';
	}
	else
	{
		switch (languages){
//			case "fr":
//				browserLangu = 'f';
//				break;
//		    case "de":
//		    	browserLangu = 'd';
//		    	break;
//		    case "it":
//		    	browserLangu = 'i';
//		    	break;
//		    case "es":
//		    	browserLangu = 's';
//		    	break;
//		    case "ja":
//		    	browserLangu = 'j';
//		    	break;
		    case "zh-tw":
			case "zh-hk":
			case "zh-mo":
			case "zh-hant":
				browserLangu = 'z';
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

adjuestlanguage();
document.write('<SCRIPT language="JavaScript" src="misc-'+browserLangu+'.js"></SCRIPT>');
