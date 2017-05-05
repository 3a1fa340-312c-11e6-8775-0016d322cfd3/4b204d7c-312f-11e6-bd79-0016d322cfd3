//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

tabArray=['USB 프린터서버','상태','설치','기타','재시작','시스템','프린터','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
//Language : Korean

//system.htm
headArray[iIndex++] = "<BR>이 페이지는 프린터서버의 일반적인 시스템 정보를 표시합니다.<BR>";
//printer.htm
headArray[iIndex++] = "<BR>이 페이지는 현재 프린터서버에 연결된 프린터의 정보를 표시합니다.<BR>참고 : 프린터에서 양방향 기능을 지원하지 않는 경우, 일부 정보가 올바르게 표시되지 않을 수 있습니다.";
//tcpip.htm
headArray[iIndex++] = "<BR>이 페이지는 프린터서버의 현재 TCP/IP 설정을 표시합니다.<BR>";
//netware.htm
headArray[iIndex++] = "<BR>이 페이지는 프린터서버의 현재 NetWare 설정을 표시합니다.<BR>";
//apple.htm
headArray[iIndex++] = "<BR>이 페이지는 프린터서버의 현재 AppleTalk 설정을 표시합니다.<BR>";
//snmp.htm
headArray[iIndex++] = "<BR>이 페이지는 프린터서버의 현재 SNMP 설정을 표시합니다.<BR>";
//Smb.htm
headArray[iIndex++] = "<BR>이 페이지는 마이크로소프트 Windows 네트워크에서의 프린터 공유 설정을 표시합니다.<BR>";
iIndex = 0;



//system.htm
textArray0[iIndex++]="시스템 정보";
textArray0[iIndex++]="프린터서버 이름 :";
textArray0[iIndex++]="시스템 접속 :";
textArray0[iIndex++]="시스템 위치 :";
textArray0[iIndex++]="시스템 최대 시간 :";
textArray0[iIndex++]="펌웨어 버전 :";
textArray0[iIndex++]="MAC 주소 :";
textArray0[iIndex++]="전자 메일 알림 :";
textArray0[iIndex++]="사용안함";
textArray0[iIndex++]="사용함";
//PRINTJOB.htm
textArray0[iIndex++]="인쇄 작업";
textArray0[iIndex++]="작업";
textArray0[iIndex++]="사용자";
textArray0[iIndex++]="경과 시간";
textArray0[iIndex++]="프로토콜";
textArray0[iIndex++]="포트";
textArray0[iIndex++]="상태";
textArray0[iIndex++]="인쇄 바이트";
textArray0[iIndex++]="작업 로그 보기";
iIndex = 0;

//Printer.htm
textArray1[iIndex++]="프린터 정보";
textArray1[iIndex++]="제조사";
textArray1[iIndex++]="모델 번호";
textArray1[iIndex++]="인쇄 지원 언어";
textArray1[iIndex++]="현재 상태";
textArray1[iIndex++]="작업 대기";
textArray1[iIndex++]="용지 부족";
textArray1[iIndex++]="오프라인";
textArray1[iIndex++]="인쇄";
iIndex = 0;

//NETWARE.htm
textArray2[iIndex++]="일반 설정";
textArray2[iIndex++]="프린터서버 이름 :";
textArray2[iIndex++]="폴링 시간 :";
textArray2[iIndex++]="초";
textArray2[iIndex++]="NetWare NDS 설정";
textArray2[iIndex++]="NDS 모드 사용 :";
textArray2[iIndex++]="사용안함";
textArray2[iIndex++]="사용함";
textArray2[iIndex++]="NDS 트리 이름 :";
textArray2[iIndex++]="NDS 컨텍스트 이름 :";
textArray2[iIndex++]="현재 상태 :";
textArray2[iIndex++]="연결 끊김";
textArray2[iIndex++]="연결됨";
textArray2[iIndex++]="NetWare Bindery 설정";
textArray2[iIndex++]="Bindery 모드 사용 :";
textArray2[iIndex++]="사용안함";
textArray2[iIndex++]="사용함";
textArray2[iIndex++]="파일 서버 이름 :";
textArray2[iIndex++]="현재 상태 :";
textArray2[iIndex++]="연결 끊김";
textArray2[iIndex++]="연결됨";
iIndex = 0;
//tcpip.htm
textArray3[iIndex++]="TCP/IP 설정";
textArray3[iIndex++]="DHCP/BOOTP 사용 :";
textArray3[iIndex++]="사용안함";
textArray3[iIndex++]="사용함";
textArray3[iIndex++]="IP 주소 :";
textArray3[iIndex++]="서브넷 마스크 :";
textArray3[iIndex++]="게이트웨이 :";
//randvoo.htm
textArray3[iIndex++]="랑데뷰 설정";
textArray3[iIndex++]="랑데뷰 설정 :";
//textArray3[iIndex++]="사용안함";
//textArray3[iIndex++]="사용함";
textArray3[iIndex++]="서비스 이름 :";
iIndex = 0;
//APPLE.htm
textArray4[iIndex++]="AppleTalk 설정";
textArray4[iIndex++]="AppleTalk :";
textArray4[iIndex++]="프린터 정보";
textArray4[iIndex++]="포트 이름 :";
textArray4[iIndex++]="프린터 유형 :";
textArray4[iIndex++]="데이터 형식 :";
iIndex = 0;
//SNMP.htm
textArray5[iIndex++]="SNMP Community 설정";
textArray5[iIndex++]="SNMP Community 1:";
textArray5[iIndex++]="읽기 전용";
textArray5[iIndex++]="읽기/쓰기";
textArray5[iIndex++]="SNMP Community 2:";
textArray5[iIndex++]="읽기 전용";
textArray5[iIndex++]="읽기/쓰기";
textArray5[iIndex++]="SNMP 트랩 설정";
textArray5[iIndex++]="SNMP 트랩 전송 :";
textArray5[iIndex++]="사용안함";
textArray5[iIndex++]="사용함";
textArray5[iIndex++]="인증 트랩 사용 :";
textArray5[iIndex++]="사용안함";
textArray5[iIndex++]="사용함";
textArray5[iIndex++]="트랩 주소 1 :";
textArray5[iIndex++]="트랩 주소 2 :";
iIndex = 0;

//JOBLOG.htm
// Translate                                  only "Refresh " is to be translated
textArray6[iIndex++]='<input type=button value=" 새로고침 " onClick="window.location.reload()">';
textArray6[iIndex++]="인쇄 작업";
textArray6[iIndex++]="작업";
textArray6[iIndex++]="사용자";
textArray6[iIndex++]="경과 시간";
textArray6[iIndex++]="프로토콜";
textArray6[iIndex++]="포트";
textArray6[iIndex++]="상태";
textArray6[iIndex++]="인쇄 바이트";
// Translate                                  only "Close " is to be translated
textArray6[iIndex++]='<input type=button value=" 닫기 " onClick="window.close()">';
iIndex = 0;

//SMB.htm
textArray7[iIndex++]="작업그룹";
textArray7[iIndex++]="이름:";
textArray7[iIndex++]="공유 이름";
textArray7[iIndex++]="프린터:";
iIndex = 0;

// Character Encoding
function CharacterEncoding()
{
	document.write('<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; charset=EUC-KR">');
}

// out of Korean
