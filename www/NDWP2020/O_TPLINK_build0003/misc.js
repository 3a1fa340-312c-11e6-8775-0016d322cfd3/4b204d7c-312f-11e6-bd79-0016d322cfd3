Len_html = 1;
Len_tab = 9;

Len_text1 = 6;	// original: 5
Len_text2 = 3;
Len_text3 = 4;
Len_text4 = 2;
Len_text5 = 2;
Len_text6 = 2;

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
				if((chromeLanguages.indexOf("zh-CN")!=-1) || 
					(chromeLanguages.indexOf("zh-SG")!=-1) || 
					(chromeLanguages.indexOf("zh-HANS")!=-1))
					browserLangu = 'c';
				else
					browserLangu = 'e';
			}
			else if(navigator.appVersion.indexOf("Safari")!=-1)
			{
				if((userAgents.indexOf("zh-cn")!=-1) || 
					(userAgents.indexOf("zh-CN")!=-1) || 
					(chromeLanguages.indexOf("zh-CN")!=-1) || 
					(userAgents.indexOf("zh-sg")!=-1) || 
					(userAgents.indexOf("zh-SG")!=-1) || 
					(chromeLanguages.indexOf("zh-SG")!=-1) || 
					(userAgents.indexOf("zh-hans")!=-1) || 
					(userAgents.indexOf("zh-HANS")!=-1) || 
					(chromeLanguages.indexOf("zh-HANS")!=-1))
					browserLangu = 'c';
				else
					browserLangu = 'e';
			}
			else if(navigator.appVersion.indexOf("Android")!=-1)
			{
				if((userAgents.indexOf("zh-cn")!=-1) || 
					(userAgents.indexOf("zh-sg")!=-1) || 
					(userAgents.indexOf("zh-hans")!=-1))
					browserLangu = 'c';
				else
					browserLangu = 'e';
			}
			else
			{
				if((userAgents.indexOf("zh-CN")!=-1) || 
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
			if(userLanguages.indexOf("zh_CN")!=-1)
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
