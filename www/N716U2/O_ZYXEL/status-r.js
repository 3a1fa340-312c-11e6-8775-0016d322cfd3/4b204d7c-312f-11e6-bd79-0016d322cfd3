//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

tabArray=['Состояние','Конфигурация','Дополнительно','Перезагрузка','Система','Принтер','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
//Language : Russian

//system.htm
headArray[iIndex++] = "<BR>На этой странице представлена общая системная информация о сервере печати.<BR>";
//printer.htm
headArray[iIndex++] = "<BR>На этой странице представлена информация о текущем принтере, подключенном к серверу печати.<BR>Внимание: Если вашим принтером не поддерживается двунаправленный обмен данными, некоторая информация не будет нормально отображаться.";
//tcpip.htm
headArray[iIndex++] = "<BR>На этой странице представлены текущие настройки TCP/IP сервера печати.<BR>";
//netware.htm
headArray[iIndex++] = "<BR>На этой странице представлены текущие настройки NetWare сервера печати.<BR>";
//apple.htm
headArray[iIndex++] = "<BR>На этой странице представлены текущие настройки AppleTalk сервера печати.<BR>";
//snmp.htm
headArray[iIndex++] = "<BR>На этой странице представлены текущие настройки SNMP сервера печати.<BR>";
//Smb.htm
headArray[iIndex++] = "<BR>На этой странице представлены текущие настройки совместного доступа к принтерам для сетей Microsoft Windows.<BR>";
iIndex = 0;

// SYSTEM.HTM
textArray0[iIndex++]="Системная информация";
textArray0[iIndex++]="Имя сервера печати:";
textArray0[iIndex++]="Ответственное лицо:";
textArray0[iIndex++]="Местоположение системы:";
textArray0[iIndex++]="Время работы:";
textArray0[iIndex++]="Версия встроенного ПО:";
textArray0[iIndex++]="MAC-адрес:";
iIndex = 0;

// PRINTER.HTM
textArray1[iIndex++]="Сведения о принтере";
textArray1[iIndex++]="Производитель";
textArray1[iIndex++]="Номер модели";
textArray1[iIndex++]="Поддерживаемые языки печати";
textArray1[iIndex++]="Текущее состояние";
iIndex = 0;

// NETWARE.HTM
textArray2[iIndex++]="Общие параметры";
textArray2[iIndex++]="Имя сервера печати:";
textArray2[iIndex++]="Время опроса:";
textArray2[iIndex++]="Настройки NetWare NDS";
textArray2[iIndex++]="Использовать режим NDS:";
textArray2[iIndex++]="Имя дерева NDS:";
textArray2[iIndex++]="Имя контекста NDS:";
textArray2[iIndex++]="Текущее состояние:";
textArray2[iIndex++]="Настройки базы объектов NetWare Bindery";
textArray2[iIndex++]="Режим базы объектов Bindery:";
textArray2[iIndex++]="Файловый сервер:";
textArray2[iIndex++]="Текущее состояние:";
iIndex = 0;

// TCPIP.HTM
textArray3[iIndex++]="Настройки TCP/IP";
textArray3[iIndex++]="Использовать DHCP/BOOTP:";
textArray3[iIndex++]="Адрес IP:";
textArray3[iIndex++]="Маска подсети:";
textArray3[iIndex++]="Шлюз:";
iIndex = 0;

// APPLE.HTM
textArray4[iIndex++]="Настройки AppleTalk";
textArray4[iIndex++]="AppleTalk:";
textArray4[iIndex++]="Сведения о принтере";
textArray4[iIndex++]="Имя порта:";
textArray4[iIndex++]="Тип принтера:";
textArray4[iIndex++]="Формат данных:";
iIndex = 0;

// SNMP.HTM
textArray5[iIndex++]="Настройки сообществ SNMP";
textArray5[iIndex++]="SNMP-сообщество 1:";
textArray5[iIndex++]="SNMP-сообщество 2:";
textArray5[iIndex++]="Настройки SNMP-отчетов";
textArray5[iIndex++]="Отправка SNMP-отчетов:";
textArray5[iIndex++]="Отчеты службы аутентификации:";
textArray5[iIndex++]="Адрес SNMP-менеджера 1:";
textArray5[iIndex++]="Адрес SNMP-менеджера 2:";
iIndex = 0;



// SMB.HTM
textArray7[iIndex++]="Рабочая группа";
textArray7[iIndex++]="Название:";
textArray7[iIndex++]="Общее название";
textArray7[iIndex++]="Принтер:";


// out of Russian
