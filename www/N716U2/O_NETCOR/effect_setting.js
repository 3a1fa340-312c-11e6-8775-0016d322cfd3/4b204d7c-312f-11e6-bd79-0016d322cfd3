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

function findcook(){
var typ;
var cook = getCurrState();
	if(cook == "1" ){
		typ="visible";
		document.getElementById("more").value ="back";
		document.getElementById("appleA").style.visibility = typ;
		document.getElementById("apple").style.visibility = typ;
		document.getElementById("appleB").style.visibility = typ;
		document.getElementById("snmpA").style.visibility = typ;
		document.getElementById("snmp").style.visibility = typ;
		document.getElementById("snmpB").style.visibility = typ;
		document.getElementById("smbA").style.visibility = typ;
		document.getElementById("smb").style.visibility = typ;
		document.getElementById("smbB").style.visibility = typ;
		document.getElementById("netwareA").style.visibility = typ;
		document.getElementById("netware").style.visibility = typ;
		document.getElementById("netwareB").style.visibility = typ;
	}else{
		typ="hidden";
		document.getElementById("more").value ="more";
		document.getElementById("appleA").style.visibility = typ;
		document.getElementById("apple").style.visibility = typ;
		document.getElementById("appleB").style.visibility = typ;
		document.getElementById("snmpA").style.visibility = typ;
		document.getElementById("snmp").style.visibility = typ;
		document.getElementById("snmpB").style.visibility = typ;
		document.getElementById("smbA").style.visibility = typ;
		document.getElementById("smb").style.visibility = typ;
		document.getElementById("smbB").style.visibility = typ;
		document.getElementById("netwareA").style.visibility = typ;
		document.getElementById("netware").style.visibility = typ;
		document.getElementById("netwareB").style.visibility = typ;
		
	}
	return false;
}
// -->
EXPRESS_MODE = 1 - getCurrState();
function MODE_CHANG(){
	var typ;
	if (EXPRESS_MODE == 1){
		typ="visible";
		document.getElementById("more").value ="back";
		document.getElementById("appleA").style.visibility = typ;
		document.getElementById("apple").style.visibility = typ;
		document.getElementById("appleB").style.visibility = typ;
		document.getElementById("snmpA").style.visibility = typ;
		document.getElementById("snmp").style.visibility = typ;
		document.getElementById("snmpB").style.visibility = typ;
		document.getElementById("smbA").style.visibility = typ;
		document.getElementById("smb").style.visibility = typ;
		document.getElementById("smbB").style.visibility = typ;
		document.getElementById("netwareA").style.visibility = typ;
		document.getElementById("netware").style.visibility = typ;
		document.getElementById("netwareB").style.visibility = typ;
		explode();
	}else{
		typ="hidden";
		document.getElementById("more").value ="more";
		document.getElementById("appleA").style.visibility = typ;
		document.getElementById("apple").style.visibility = typ;
		document.getElementById("appleB").style.visibility = typ;
		document.getElementById("snmpA").style.visibility = typ;
		document.getElementById("snmp").style.visibility = typ;
		document.getElementById("snmpB").style.visibility = typ;
		document.getElementById("smbA").style.visibility = typ;
		document.getElementById("smb").style.visibility = typ;
		document.getElementById("smbB").style.visibility = typ;
		document.getElementById("netwareA").style.visibility = typ;
		document.getElementById("netware").style.visibility = typ;
		document.getElementById("netwareB").style.visibility = typ;
        contract();
	}
	EXPRESS_MODE=1-EXPRESS_MODE;
	
	return true;
}

function MODE_CHANG_TRUE(){
	var typ;
	EXPRESS_MODE = 1;

	typ="visible";
	document.getElementById("more").value ="back";
	document.getElementById("appleA").style.visibility = typ;
	document.getElementById("apple").style.visibility = typ;
	document.getElementById("appleB").style.visibility = typ;
	document.getElementById("snmpA").style.visibility = typ;
	document.getElementById("snmp").style.visibility = typ;
	document.getElementById("snmpB").style.visibility = typ;
	document.getElementById("smbA").style.visibility = typ;
	document.getElementById("smb").style.visibility = typ;
	document.getElementById("smbB").style.visibility = typ;
	document.getElementById("netwareA").style.visibility = typ;
	document.getElementById("netware").style.visibility = typ;
	document.getElementById("netwareB").style.visibility = typ;
	explode();

	EXPRESS_MODE=1-EXPRESS_MODE;
	
	return true;
}


	    






	    