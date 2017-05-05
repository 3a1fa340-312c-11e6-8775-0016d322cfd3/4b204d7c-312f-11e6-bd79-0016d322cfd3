
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Korean
//tabArray=['USB&nbsp;프린터서버','상태','설치','기타','재시작','시스템','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];
tabArray=['USB&nbsp;프린터서버','상태','설치','기타','재시작','시스템','서비스','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>이 설치 페이지에서 프린터서버의 일반적인 시스템 설정을 구성할 수 있습니다.<br>";
//ctcpip.htm
headArray[iIndex++] = "<BR>이 설치 페이지에서 프린터서버의 TCP/IP설정을 구성할 수 있습니다.";
//cnetware.htm
headArray[iIndex++] = "<BR>이 설치 페이지에서 프린터서버의 NetWare 기능을 구성할 수 있습니다.";
//capple.htm 
headArray[iIndex++] ="<BR>이 설치 페이지에서 프린터서버의 AppleTalk 설정을 구성할 수 있습니다.<br>";
//csnmp.htm
headArray[iIndex++] ="<BR>이 설치 페이지에서 프린터서버의 SNMP 설정을 구성할 수 있습니다.";
//csmp.htm
headArray[iIndex++] ="<BR>이 페이지는 마이크로소프트 Windows 네트워크에서의 프린터 공유 설정을 표시합니다.";
iIndex = 0;

//csystem.htm
textArray0[iIndex++]="전자 메일 알림 설정";
textArray0[iIndex++]="전자 메일 알림 :";
textArray0[iIndex++]="사용안함";
textArray0[iIndex++]="사용함";
textArray0[iIndex++]="SMTP 서버 IP 주소 :";
textArray0[iIndex++]="관리자 전자 메일 주소 :";
textArray0[iIndex++]="시스템 설정";
textArray0[iIndex++]="프린터서버 이름 :";
textArray0[iIndex++]="시스템 접속 :";

//textArray0[iIndex++]="<INPUT TYPE=TEXT NAME='SnmpSysContact' SIZE='20' MAXLENGTH='15' VALUE=";
//textArray0[iIndex++]='ZO__I-SnmpContact';
//textArray0[iIndex++]=">";

textArray0[iIndex++]="시스템 위치 :";

//textArray0[iIndex++]="<INPUT TYPE=TEXT NAME='SnmpSysLocation' SIZE='20' MAXLENGTH='24' VALUE="
//textArray0[iIndex++]='ZO__I-SnmpLocation';
//textArray0[iIndex++]=">";

textArray0[iIndex++]="관리자 비밀번호";
//textArray0[iIndex++]="Account :";
//textArray0[iIndex++]="admin";
textArray0[iIndex++]="비밀번호 :";
textArray0[iIndex++]="비밀번호 확인 :";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="저장 & 재시작" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
//CPRINTER.htm
textArray1[iIndex++]="병렬 포트 - 양방향 설정";
textArray1[iIndex++]="인쇄 포트 1 :";
textArray1[iIndex++]='<input type="button" value="저장 & 재시작" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
//CTCPIP.htm
textArray2[iIndex++]="TCP/IP 설정";
textArray2[iIndex++]="TCP/IP 설정 자동 할당 (DHCP/BOOTP 사용)";
textArray2[iIndex++]="다음 TCP/IP 설정 사용";
textArray2[iIndex++]="IP 주소 :";
textArray2[iIndex++]="서브넷 마스크 :";
textArray2[iIndex++]="기본 라우터 :";
//crandvoo.htm
textArray2[iIndex++]="랑데뷰 설정";
textArray2[iIndex++]="랑데뷰 설정 :";
//textArray2[iIndex++]="사용안함";
//textArray2[iIndex++]="사용함";
textArray2[iIndex++]="서비스 이름 :";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="저장 & 재시작" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="AppleTalk 설정";
textArray3[iIndex++]="AppleTalk 영역 :";
textArray3[iIndex++]="포트 이름 :";
textArray3[iIndex++]="프린터 구성";
textArray3[iIndex++]="유형 :";
textArray3[iIndex++]="바이너리 프로토콜 :";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="저장 & 재시작" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
//CSNMP.htm
textArray4[iIndex++]="SNMP Community 설정";
textArray4[iIndex++]="HP WebJetAdmin 지원 :";
textArray4[iIndex++]="사용안함";
textArray4[iIndex++]="사용함";
textArray4[iIndex++]="SNMP Community 이름 1 :";
textArray4[iIndex++]="특전 :";
textArray4[iIndex++]="Read-Only";
textArray4[iIndex++]="Read-Write";
textArray4[iIndex++]="SNMP Community 이름 2 :";
textArray4[iIndex++]="특전 :";
textArray4[iIndex++]="Read-Only";
textArray4[iIndex++]="Read-Write";
textArray4[iIndex++]="SNMP 트랩 설정";
textArray4[iIndex++]="SNMP 트랩 전송 :";
textArray4[iIndex++]="사용안함";
textArray4[iIndex++]="사용함";
textArray4[iIndex++]="인증 트랩 사용 :";
textArray4[iIndex++]="사용안함";
textArray4[iIndex++]="사용함";
textArray4[iIndex++]="트랩 주소 1 :";
textArray4[iIndex++]="트랩 주소 2 :";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="저장 & 재시작" onClick="return SaveSetting(';
// Begin don't translate
textArray4[iIndex++]="'RESTART.HTM');";
textArray4[iIndex++]='">';
iIndex = 0;
// End don't translate
//keyhelp.htm
//textArray5[iIndex++]="<b>WEP Key Format</b>";
//textArray5[iIndex++]="An alphanumeric character is 'a' through 'z', 'A' through 'Z', and '0' through '9'.";
//textArray5[iIndex++]="A hexadecimal digit is '0' through '9' and 'A' through 'F'.";
//textArray5[iIndex++]="Depending on the key format you select:";
//textArray5[iIndex++]="For 64-bit (sometimes called 40-bit) WEP encryption, enter the key which contains 5 alphanumeric characters or 10 hexadecimal digits. For example: AbZ12 (alphanumeric format) or ABCDEF1234 (hexadecimal format).";
//textArray5[iIndex++]="For 128-bit WEP encryption, enter the key which contains 13 alphanumeric characters or 26 hexadecimal digits.";
// Translate                                  only "Close" is to be translated
textArray5[iIndex++]='<INPUT TYPE=button value=" 닫기 " onClick="window.close()">';
iIndex = 0;
//browser.htm
//textArray6[iIndex++]="SSID";
//textArray6[iIndex++]="AP's MAC Address or BSSID";
//textArray6[iIndex++]="Channel";
//textArray6[iIndex++]="Type";
//textArray6[iIndex++]="WEP/WPA-PSK";
//textArray6[iIndex++]="Signal Strength";
// Translate                                  only "Rescan" is to be translated
//textArray6[iIndex++]='<INPUT TYPE=submit VALUE="Rescan">';
// Translate                                  only "Close" is to be translated
//textArray6[iIndex++]='<INPUT TYPE=button VALUE=" Close " onClick="window.close()">';
//iIndex = 0;
// End don't translate
// ERROR.htm
textArray7[iIndex++]="오류";
textArray7[iIndex++]="잘못된 IP 주소";
textArray7[iIndex++]="잘못된 서브넷 마스크 주소";
textArray7[iIndex++]="잘못된 게이트웨이 주소";
textArray7[iIndex++]="잘못된 폴링 시간값";
textArray7[iIndex++]="잘못된 프린터서버 이름";
textArray7[iIndex++]="잘못된 파일 서버 이름";
textArray7[iIndex++]="DHCP/BOOTP 서버를 찾을 수 없습니다.";
textArray7[iIndex++]="잘못된 SNMP 트랩 IP 주소";
textArray7[iIndex++]="설치 비밀번호와 확인 비밀번호가 일치하지 않습니다.";
textArray7[iIndex++]="잘못된 펌웨어 또는 업그레이드 실패";
textArray7[iIndex++]="Failed in importing the CFG file";
textArray7[iIndex++]="";
textArray7[iIndex++]="뒤로가기";
iIndex = 0;

// CNETWARE.htm
textArray8[iIndex++]="일반 설정";
textArray8[iIndex++]="프린터서버 이름 :";
textArray8[iIndex++]="폴링 시간 :";
textArray8[iIndex++]="&nbsp;초 (최소 : 3, 최대 :  29초)";
textArray8[iIndex++]="로그온 비밀번호 :";
textArray8[iIndex++]="NetWare NDS 설정";
textArray8[iIndex++]="NDS 모드 사용 :";
textArray8[iIndex++]="사용안함";
textArray8[iIndex++]="사용함";
textArray8[iIndex++]="NDS 트리 이름 :";
textArray8[iIndex++]="NDS 컨텍스트 이름 :";
textArray8[iIndex++]="NetWare Bindery 설정";
textArray8[iIndex++]="Bindery 모드 사용 :";
textArray8[iIndex++]="사용안함";
textArray8[iIndex++]="사용함";
textArray8[iIndex++]="파일 서버 이름 :";
textArray8[iIndex++]="<option>Not found!</option>";
textArray8[iIndex++]='<input type="button" value="저장 & 재시작" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.htm
textArray9[iIndex++]="작업그룹";
textArray9[iIndex++]="이름:";
textArray9[iIndex++]="공유 이름";
textArray9[iIndex++]="프린터:";
textArray9[iIndex++]='<input type="button" value="저장 & 재시작" onClick="return SaveSetting(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;

//CSERVICES.HTM
textArray10[iIndex++]="서비스";
textArray10[iIndex++]="Telnet:";
textArray10[iIndex++]="사용안함";
textArray10[iIndex++]="사용함";
textArray10[iIndex++]="HTTP:";
textArray10[iIndex++]="사용안함";
textArray10[iIndex++]="사용함";
textArray10[iIndex++]='<input type="button" value="저장 & 재시작" onClick="return SaveServices(';
textArray10[iIndex++]="'RESTART.HTM');";
textArray10[iIndex++]='">';
iIndex = 0;

// functions
// CSYSTEM.HTM
function CheckPwd(szURL)
{
 	if(document.forms[0].SetupPWD.value != document.forms[0].ConfirmPWD.value && document.forms[0].SetupPWD.value != "ZO__I-SetupPassword" )
	{
		alert("Administrator's Password and Re-type Password do not match !");
		return false;
	}

	if(document.forms[0].SetupPWD.value.match('"')||document.forms[0].SetupPWD.value.match("'"))
	{
		alert("Administrator's Password not allowed below character !\n\'   \" ");
		return false;
	}

	document.forms[0].action=szURL;
	document.forms[0].submit();
	return false;
}

// CPRINTER.HTM

// CTCPTP.HTM

// CNETWARE.HTM

// CAPPLE.HTM

// CSNMP.HTM

// CSMB.HTM
function CheckSMB(szURL)
{
	if(document.forms[0].SMBWorkGroup.value == '')
	{
		alert("ERROR! The workgroup name cannot be empty!");
		return false;
	}
	else
	{
		if (document.forms[0].SMBPrint1.value == '')
		{		
			alert("ERROR! The SMB shared printer name cannot be empty!");
		 	return false; 
		}
		else
		{
			document.forms[0].action=szURL;
			document.forms[0].submit(); 
			return false; 
		}
	}
}

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=EUC-KR">');
}

// out of Korean
