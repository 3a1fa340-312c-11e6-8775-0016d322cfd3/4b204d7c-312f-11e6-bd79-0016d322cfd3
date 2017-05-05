

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Korean
tabArray=['USB&nbsp;프린터서버','상태','설치','기타','재시작','공장 기본값','펌웨어 업그레이드'];

	//upgrade.htm
headArray[iIndex++] ="<br>이 페이지에서 프린터서버의 펌웨어를 업그레이드하실 수 있습니다.<br>참고 : 진행하시기 전에 펌웨어가 올바른지 확인하시기 바랍니다. 사용해야할 펌웨어 파일을 모르시면, 해당 지역의 대리점에 기술지원 문의를 하시기 바랍니다.";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "<b>공장 기본값</b>을 클릭 후 <b>확인</b>을 클릭하셔서 프린터서버의 모든 기본 설정을 다시 로드하십시오. 경고! 현재 모든 설정은 삭제됩니다.";
textArray1[iIndex++] = "<b>펌웨어 업그레이드</b> 의를 클릭하셔서 펌웨어 디렉토리를 탐색하시고 새로운 펌웨어로 프린터서버를 다시 로드하십시오.";
//textArray1[iIndex++] = "You can still connect the print server with the current IP address after it restarts.";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;확인&nbsp;&nbsp;" onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="펌웨어 업그레이드";
textArray2[iIndex++]="펌웨어 디렉토리/파일 선택:";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="펌웨어 업그레이드" onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="이 페이지에서 프린터서버를 재시작할 수 있습니다.<br>";
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>프린터서버 재시작</B></FONT><br><br>설정을 저장하고 프린터서버를 재시작하시겠습니까?<br><br>";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;확인&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

	//재시작.htm
textArray4[iIndex++] = "재시작중 …";
textArray4[iIndex++] = "프린터서버가 재시작하는 동안 기다리시기 바랍니다.";
iIndex = 0;
	//urestart.htm
textArray5[iIndex++] = "업그레이드 완료!";
textArray5[iIndex++] = "펌웨어 업그레이드 후, 프린터서버가 자동으로 재시작하오니, 잠시만 기다려주시기 바랍니다. ";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "공장 기본값 로딩중 …";
textArray6[iIndex++] = "기본 설정 로드 후, 프린터서버가 자동으로 재시작합니다.<BR><BR>프린터서버가 재시작되면 상태 페이지로 되돌아옵니다.";
iIndex = 0;

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=EUC-KR">');
}

// out of Korean
