Len_html = 8;
Len_tab = 12;

Len_text0 = 18;		// CSYSTEM.HTM
Len_text1 = 5;
Len_text2 = 12;
Len_text3 = 9;
Len_text4 = 24;		// CSNMP.HTM
Len_text5 = 7;
Len_text6 = 8;
Len_text7 = 14;		// ERROR.HTM
Len_text8 = 20;		// CNETWARE.HTM
Len_text9 = 7;		// CSMB.HTM
Len_text10 = 20;	// CWLAN.HTM Basic Settings		// original:10
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

function SaveSetting(szURL)
{
	document.forms[0].action=szURL;
	document.forms[0].submit();
	return false;
}

function SaveSettingCWLAN(szURL)
{
	var f = document.myform;
	var iln, ch;

	// Basic Settings
	// Wireless Mode
	
	// Advanced Settings
	// Security Mode
	if( f.security_mode[1].checked )
	{
		// WEP 64-bit or WEP 128-bit
		var hs;
	
		var iWEP_sel;			// Key Index
		var iWEP_type;			// WEP Encryption
		
		if (f.wep_sel[0].checked)
		{
			iWEP_sel = 0;
			
			if(f.wep_key1.value == '')
			{
				alert("WEP Key cannot be empty!");
				return false;
			}
		}
		if (f.wep_sel[1].checked)
		{
			iWEP_sel = 1;
			
			if(f.wep_key2.value == '')
			{
				alert("WEP Key cannot be empty!");
				return false;
			}
		}
		if (f.wep_sel[2].checked)
		{
			iWEP_sel = 2;
			
			if(f.wep_key3.value == '')
			{
				alert("WEP Key cannot be empty!");
				return false;
			}
		}
		if (f.wep_sel[3].checked)
		{
			iWEP_sel = 3;
			
			if(f.wep_key4.value == '')
			{
				alert("WEP Key cannot be empty!");
				return false;
			}
		}
		
		f.WLWEPKeySel.value = iWEP_sel;

		iWEP_type = f.wep_type.selectedIndex;
		// [0]: 64-bit 10 hex digits	[1]: 64-bit 5 characters
		// [2]: 128-bit 26 hex digits	[3]: 128-bit 13 characters

		// 64-bit hex or 128-bit hex
		if((iWEP_type == 0) || (iWEP_type == 2))
		{
			f.WLWEPFormat.value = 1;	// Hexadecimal

			switch(iWEP_sel)
			{
			case 0:			// WEP key 1
				hs = convertHexString (document.myform.wep_key1.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP key 1\" is not valid!");
					return false;
				}
				break;
			case 1:			// WEP key 2
				hs = convertHexString (document.myform.wep_key2.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP key 2\" is not valid!");
					return false;
				}
				break;
			case 2:			// WEP key 3
				hs = convertHexString (document.myform.wep_key3.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP key 3\" is not valid!");
					return false;
				}
				break;
			case 3:			// WEP key 4
				hs = convertHexString (document.myform.wep_key4.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP key 4\" is not valid!");
					return false;
				}
				break;
			default:		// WEP key 1
				hs = convertHexString (document.myform.wep_key1.value,(iWEP_type) ? (13) : (5),13);
				if (!hs) {
					alert ("\"WEP key 1\" is not valid!");
					return false;
				}
			}
		}	// end of if((iWEP_type == 0) || (iWEP_type == 2))
		
		// 64-bit Alphanumeric or 128-bit Alphanumeric
		if((iWEP_type == 1) || (iWEP_type == 3))
		{
			f.WLWEPFormat.value = 0;		// Alphanumeric
			
			switch(iWEP_sel)
			{
			case 0:			// WEP key 1
				for ( iln = 0; iln < f.wep_key1.value.length; iln++ )
				{
			    	ch = f.wep_key1.value.charAt(iln);
					
				  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '!') || (ch == '?') )
				  		continue;
				  	else
				  	{
				  		alert ("\"WEP key 1\" is not valid!");
				  		return false;
				  	}
				}
				break;
			case 1:			// WEP key 2
				for ( iln = 0; iln < f.wep_key2.value.length; iln++ )
				{
			    	ch = f.wep_key2.value.charAt(iln);

				  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '!') || (ch == '?') )
				  		continue;
				  	else
				  	{
				  		alert ("\"WEP key 2\" is not valid!");
				  		return false;
				  	}
				}
				break;
			case 2:			// WEP key 3
				for ( iln = 0; iln < f.wep_key3.value.length; iln++ )
				{
			    	ch = f.wep_key3.value.charAt(iln);
					
				  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '!') || (ch == '?') )
				  		continue;
				  	else
				  	{
				  		alert ("\"WEP key 3\" is not valid!");
				  		return false;
				  	}
				}
				break;
			case 3:			// WEP key 4
				for ( iln = 0; iln < f.wep_key4.value.length; iln++ )
				{
			    	ch = f.wep_key4.value.charAt(iln);
					
				  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '!') || (ch == '?') )
				  		continue;
				  	else
				  	{
				  		alert ("\"WEP key 4\" is not valid!");
				  		return false;
				  	}
				}
				break;
			default:		// WEP key 1
				for ( iln = 0; iln < f.wep_key1.value.length; iln++ )
				{
			    	ch = f.wep_key1.value.charAt(iln);
					
				  	if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '!') || (ch == '?') )
				  		continue;
				  	else
				  	{
				  		alert ("\"WEP key 1\" is not valid!");
				  		return false;
				  	}
				}
			}
		}
		
		f.WLWEPType.value = 1;	// default 1: WEP 64-bit

		if((f.wep_type.options[0].selected) || (f.wep_type.options[1].selected))
		{
			f.WLWEPType.value = 1;	// WEP 64-bit

			f.WLWEPKey1.value = f.wep_key1.value;
			f.WLWEPKey2.value = f.wep_key2.value;
			f.WLWEPKey3.value = f.wep_key3.value;
			f.WLWEPKey4.value = f.wep_key4.value;
		}

		if((f.wep_type.options[2].selected) || (f.wep_type.options[3].selected))
		{
			// WEP 128-bit
			f.WLWEPType.value = 2;
			
			f.WLWEP128Key.value = f.wep_key1.value;
			f.WLWEP128Key2.value = f.wep_key2.value;
			f.WLWEP128Key3.value = f.wep_key3.value;
			f.WLWEP128Key4.value = f.wep_key4.value;
		}

		// Authentication
		if(f.wep_authmode[0].selected)
			f.WLAuthType.value = 1;		// Open System
		if(f.wep_authmode[1].selected)
			f.WLAuthType.value = 2;		// Shared Key
	}
	else if( f.security_mode[2].checked )
	{
		// WPA-PSK
		f.WLAuthType.value = 4;

		if(!validate_Preshared())
			return false;
	}
	else if( f.security_mode[3].checked )
	{
		// WPA2-PSK
		f.WLAuthType.value = 5;

		if(!validate_Preshared())
			return false;
	}
	else
	{
		// Disabled
		f.WLWEPType.value = 0;
		f.WLAuthType.value = 1;
	}

	document.forms[0].action=szURL;
	document.forms[0].submit();
	return false; 
}

function checkPreSharedKey(szURL)
{
	var theForm = document.forms[0];

	if( theForm.WLAuthType.value == "4" )
	{
		if( theForm.WPAPASS.value.length < 8 )
		{
			alert("8 to 63 characters!");
			return false;
		}
		else if( theForm.WPAPASS.value.indexOf(" ") >= 0 )
		{
			alert("Don't input the space character!");
			return false;
		}
		else
		{
			theForm.action=szURL;
			theForm.submit();
			return false;
		}
	}
	else
	{
		theForm.action=szURL;
		theForm.submit();
		return false;
	}
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
