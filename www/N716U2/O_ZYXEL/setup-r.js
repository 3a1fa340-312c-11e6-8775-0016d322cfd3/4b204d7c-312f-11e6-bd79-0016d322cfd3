
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Russian
tabArray=['Состояние','Конфигурация','Дополнительно','Перезагрузка','Система','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>Эта страница позволяет настроить общие системные параметры сервера печати.<br>";
//ctcpip.htm
headArray[iIndex++] = "<BR>Эта страница позволяет настроить параметры TCP/IP сервера печати.";
//cnetware.htm
headArray[iIndex++] = "<BR>Эта страница позволяет настроить параметры NetWare сервера печати.";
//capple.htm 
headArray[iIndex++] ="<BR>Эта страница позволяет настроить параметры AppleTalk сервера печати.<br>";
//csnmp.htm
headArray[iIndex++] ="<BR>Эта страница позволяет настроить параметры SNMP сервера печати.";
//csmp.htm
headArray[iIndex++] ="<BR>На этой странице представлены текущие настройки совместного доступа к принтерам для сетей Microsoft Windows.";
iIndex = 0;

// CSYSTEM.HTM
textArray0[iIndex++]="Системные параметры";
textArray0[iIndex++]="Имя сервера печати:";
textArray0[iIndex++]="Ответственное лицо:";
textArray0[iIndex++]="Местоположение системы:";
textArray0[iIndex++]="Пароль администратора";
textArray0[iIndex++]="Пароль:";
textArray0[iIndex++]="Повторите пароль:";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="Сохранить и перезагрузить систему" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
// CPRINTER.HTM
textArray1[iIndex++]="Параллельный порт - параметры двустороннего обмена";
textArray1[iIndex++]="Порт принтера 1:";
textArray1[iIndex++]='<input type="button" value="Сохранить и перезагрузить систему" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
// CTCPIP.HTM
textArray2[iIndex++]="Настройки TCP/IP";
textArray2[iIndex++]="Загружать настройки TCP/IP автоматически (из таблицы DHCP/BOOTP)";
textArray2[iIndex++]="Использовать следующие настройки TCP/IP:";
textArray2[iIndex++]="Адрес IP:";
textArray2[iIndex++]="Маска подсети:";
textArray2[iIndex++]="Маршрутизатор по умолчанию:";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="Сохранить и перезагрузить систему" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="Настройки AppleTalk";
textArray3[iIndex++]="Зона AppleTalk:";
textArray3[iIndex++]="Имя порта:";
textArray3[iIndex++]="Конфигурация принтера";
textArray3[iIndex++]="Тип:";
textArray3[iIndex++]="Двоичный протокол:";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="Сохранить и перезагрузить систему" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
// CSNMP.HTM
textArray4[iIndex++]="Настройки сообществ SNMP:";
textArray4[iIndex++]="Поддержка службы HP WebJetAdmin:";
textArray4[iIndex++]="Имя SNMP-сообщества 1:";
textArray4[iIndex++]="Уровень доступа:";
textArray4[iIndex++]="Имя SNMP-сообщества 2:";
textArray4[iIndex++]="Уровень доступа:";
textArray4[iIndex++]="Настройки SNMP-отчетов";
textArray4[iIndex++]="Отправка SNMP-отчетов:";
textArray4[iIndex++]="Отчеты службы аутентификации:";
textArray4[iIndex++]="Адрес SNMP-менеджера 1:";
textArray4[iIndex++]="Адрес SNMP-менеджера 2:";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="Сохранить и перезагрузить систему" onClick="return SaveSetting(';
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
textArray5[iIndex++]='<INPUT TYPE=button VALUE=" Close " onClick="window.close()">';
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
textArray7[iIndex++]="ОШИБКА";
textArray7[iIndex++]="Вернуться";
iIndex = 0;

// CNETWARE.HTM
textArray8[iIndex++]="Общие параметры";
textArray8[iIndex++]="Имя сервера печати:";
textArray8[iIndex++]="Время опроса:";
textArray8[iIndex++]="&nbsp;время в секундах (мин: 3 сек, макс: 29 сек)";
textArray8[iIndex++]="Пароль доступа:";
textArray8[iIndex++]="Настройки NetWare NDS";
textArray8[iIndex++]="Использовать режим NDS:";
textArray8[iIndex++]="Имя дерева NDS:";
textArray8[iIndex++]="Имя контекста NDS:";
textArray8[iIndex++]="Настройки базы объектов NetWare Bindery";
textArray8[iIndex++]="Режим базы объектов Bindery:";
textArray8[iIndex++]="Файловый сервер:";
textArray8[iIndex++]='<input type="button" value="Сохранить и перезагрузить систему" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.HTM
textArray9[iIndex++]="Рабочая группа";
textArray9[iIndex++]="Название:";
textArray9[iIndex++]="Общее название";
textArray9[iIndex++]="Принтер:";
textArray9[iIndex++]='<input type="button" value="Сохранить и перезагрузить систему" onClick="return SaveSetting(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;
// out of Russian
