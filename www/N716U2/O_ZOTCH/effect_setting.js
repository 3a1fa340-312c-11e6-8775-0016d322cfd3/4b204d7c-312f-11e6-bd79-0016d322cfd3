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

function findcook()
{
	var typ;
	var cook = getCurrState();
	
	if(cook == "1" ){
		document.getElementById("BtnMore").style.display = '';
		document.getElementById("BtnLess").style.display = 'none';
		document.getElementById("itemMenu").style.display = '';
		document.getElementById("itemMenu2").style.display = 'none';
	}else{
		document.getElementById("BtnMore").style.display = 'none';
		document.getElementById("BtnLess").style.display = '';
		document.getElementById("itemMenu").style.display = '';
		document.getElementById("itemMenu2").style.display = '';
		
	}
	return false;
}
// -->
EXPRESS_MODE = 1 - getCurrState();
function MODE_CHANG()
{
	var typ;

	if (EXPRESS_MODE == 1){
		typ="visible";
		document.getElementById("BtnMore").style.display = '';
		document.getElementById("BtnLess").style.display = 'none';
		document.getElementById("itemMenu").style.display = '';
		document.getElementById("itemMenu2").style.display = 'none';
		explode();
	}else{
		typ="hidden";
		document.getElementById("BtnMore").style.display = 'none';
		document.getElementById("BtnLess").style.display = '';
		document.getElementById("itemMenu").style.display = '';
		document.getElementById("itemMenu2").style.display = '';
        contract();
	}
	EXPRESS_MODE=1-EXPRESS_MODE;
	
	return true;
}

// JavaScript Document mm.js
function MM_swapImgRestore() { //v3.0
  var i,x,a=document.MM_sr; for(i=0;a&&i<a.length&&(x=a[i])&&x.oSrc;i++) x.src=x.oSrc;
}

function MM_preloadImages() { //v3.0
  var d=document; if(d.images){ if(!d.MM_p) d.MM_p=new Array();
    var i,j=d.MM_p.length,a=MM_preloadImages.arguments; for(i=0; i<a.length; i++)
    if (a[i].indexOf("#")!=0){ d.MM_p[j]=new Image; d.MM_p[j++].src=a[i];}}
}

function MM_findObj(n, d) { //v4.01
  var p,i,x;  if(!d) d=document; if((p=n.indexOf("?"))>0&&parent.frames.length) {
    d=parent.frames[n.substring(p+1)].document; n=n.substring(0,p);}
  if(!(x=d[n])&&d.all) x=d.all[n]; for (i=0;!x&&i<d.forms.length;i++) x=d.forms[i][n];
  for(i=0;!x&&d.layers&&i<d.layers.length;i++) x=MM_findObj(n,d.layers[i].document);
  if(!x && d.getElementById) x=d.getElementById(n); return x;
}

function MM_swapImage() { //v3.0
  var i,j=0,x,a=MM_swapImage.arguments; document.MM_sr=new Array; for(i=0;i<(a.length-2);i+=3)
   if ((x=MM_findObj(a[i]))!=null){document.MM_sr[j++]=x; if(!x.oSrc) x.oSrc=x.src; x.src=a[i+2];}
}
	    