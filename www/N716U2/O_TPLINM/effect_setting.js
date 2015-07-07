<!--
var current=getCurrState();

function getCurrState() {
  var label = "currState=";
  var labelLen = label.length;
  var cLen = document.cookie.length;
  var i = 0;
  while (i < cLen) {
    var j = i + labelLen;
    if (document.cookie.substring(i,j) == label) {
      var cEnd = document.cookie.indexOf(";",j);
      if (cEnd == -1) { cEnd = document.cookie.length; }
      return unescape(document.cookie.substring(j,cEnd));
    }
    i++;
  }
  return "";
}

function setCurrState(setting) {
  var expire = new Date();
  expire.setTime(expire.getTime() + ( 7*24*60*60*1000 ) );
  document.cookie = "currState=" + escape(setting) + "; expires=" + expire.toGMTString();
}


function explode() {
  current = "";
  initState="";
  for (var i = 1; i < 2; i++) { 
    initState += "1";
    current += "1";
  }
  setCurrState(initState);
}
function contract() {
  current = "";
  initState="";
  for (var i = 1; i < 2; i++) { 
    initState += 0;
    current += 0;
  }
  setCurrState(initState);
}

function itslanguage()
{
	var languages = navigator.browserLanguage;
	var userAgents = navigator.userAgent;
	var userLanguages = navigator.userLanguage;

	var rc;

	if(navigator.appName=="Netscape")
	{
		if(navigator.appVersion.indexOf("5.0")!=-1)
		{
			if(navigator.appVersion.indexOf("Safari")!=-1)
			{
				if(userAgents.indexOf("zh-cn")!=-1)
					rc = 'c';
				else
					rc = 'e';
			}
			else
			{
				if(userAgents.indexOf("zh-CN")!=-1)
					rc = 'c';
				else
					rc = 'e';
			}
		}
		else
			rc = 'e';
	}
	else if(navigator.appName=="Konqueror")
	{
		if(navigator.appVersion.indexOf("5.0")!=-1)
		{
			if(userLanguages.indexOf("zh_CN")!=-1)
				rc = 'c';
			else
				rc = 'e';
		}
		else
			rc = 'e';
	}
	else
	{
		switch (languages){
			case "zh-cn":
				rc = 'c';
				break;
		    default:
		    	rc = 'e';
		}
	}
	
	return rc;
}

function findcook()
{
	var typ;
	var cook = getCurrState();
	var itslanguageRC;
	
	itslanguageRC = itslanguage();
	
	itslanguageRC = 'c';

	if(cook == "1" ){
		typ="visible";
		if(itslanguageRC == 'c')
			document.getElementById("more").value ="隐藏";
		else
			document.getElementById("more").value ="Hide";
		document.getElementById("apple").style.visibility = typ;
		document.getElementById("snmp").style.visibility = typ;
		document.getElementById("smb").style.visibility = typ;
		document.getElementById("netware").style.visibility = typ;
	}else{
		typ="hidden";
		if(itslanguageRC == 'c')
			document.getElementById("more").value ="更多";
		else
			document.getElementById("more").value ="More";
		document.getElementById("apple").style.visibility = typ;
		document.getElementById("snmp").style.visibility = typ;
		document.getElementById("smb").style.visibility = typ;
		document.getElementById("netware").style.visibility = typ;
		
	}
	return false;
}
// -->
EXPRESS_MODE = 1 - getCurrState();
function MODE_CHANG()
{
	var typ;
	var itslanguageRC;
	
	itslanguageRC = itslanguage();
	
	itslanguageRC = 'c';

	if (EXPRESS_MODE == 1){
		typ="visible";
		if(itslanguageRC == 'c')
			document.getElementById("more").value ="隐藏";
		else
			document.getElementById("more").value ="Hide";
		document.getElementById("apple").style.visibility = typ;
		document.getElementById("snmp").style.visibility = typ;
		document.getElementById("smb").style.visibility = typ;
		document.getElementById("netware").style.visibility = typ;
		explode();
	}else{
		typ="hidden";
		if(itslanguageRC == 'c')
			document.getElementById("more").value ="更多";
		else
			document.getElementById("more").value ="More";
		document.getElementById("apple").style.visibility = typ;
		document.getElementById("snmp").style.visibility = typ;
		document.getElementById("smb").style.visibility = typ;
		document.getElementById("netware").style.visibility = typ;
        contract();
	}
	EXPRESS_MODE=1-EXPRESS_MODE;
	
	return true;
}


	    






	    