Len_html = 14;	// original:7
Len_menu = 6;
Len_tab = 7;

Len_text0 = 17;
Len_text1 = 5;
Len_text2 = 12;
Len_text3 = 8;
Len_text4 = 6;
Len_text5 = 8;
Len_text6 = 10;
Len_text7 = 4;

htmArray = new Array(Len_html);
menuArray = new Array(Len_menu);
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

htmArray = ['system0','system','printer0','printer','tcpip0','tcpip','netware0','netware','apple0','apple','snmp0','snmp','smb0','smb'];
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
				
				if(userAgents.indexOf("fr")!=-1)
					browserLangu = 'f';
				else if(userAgents.indexOf("de-de")!=-1)
					browserLangu = 'd';
				else if(userAgents.indexOf("it-it")!=-1)
					browserLangu = 'i';
				else if(userAgents.indexOf("es")!=-1)
					browserLangu = 's';
				else if(userAgents.indexOf("ja-jp")!=-1)
					browserLangu = 'j';
				else
					browserLangu = 'e';
			}
			else
			{
				if(userAgents.indexOf("fr-FR")!=-1)
					browserLangu = 'f';		
				else if(userAgents.indexOf("de-DE")!=-1)
					browserLangu = 'd';
				else if(userAgents.indexOf("de-AT")!=-1)
					browserLangu = 'd';
				else if(userAgents.indexOf("it-IT")!=-1)
					browserLangu = 'i';
				else if(userAgents.indexOf("es-ES")!=-1)
					browserLangu = 's';
				else if(userAgents.indexOf("ja-JP")!=-1)
					browserLangu = 'j';
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
			if(userLanguages.indexOf("fr")!=-1)
				browserLangu = 'f';
			else if(userLanguages.indexOf("de")!=-1)
				browserLangu = 'd';
			else if(userLanguages.indexOf("it")!=-1)
				browserLangu = 'i';
			else if(userLanguages.indexOf("es")!=-1)
				browserLangu = 's';
			else if(userLanguages.indexOf("ja")!=-1)
				browserLangu = 'j';
			else
				browserLangu = 'e';
		}
		else
			browserLangu = 'e';
	}
	else
	{
		switch (languages){
			case "fr":
				browserLangu = 'f';
				break;
		    case "de":
		    	browserLangu = 'd';
		    	break;
		    case "it":
		    	browserLangu = 'i';
		    	break;
		    case "es":
		    	browserLangu = 's';
		    	break;
		    case "ja":
		    	browserLangu = 'j';
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

function showmenu(iPoision)
{
	document.write(menuArray[iPoision]);
	return true;
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
document.write('<SCRIPT language="JavaScript" src="status-c.js"></SCRIPT>');
