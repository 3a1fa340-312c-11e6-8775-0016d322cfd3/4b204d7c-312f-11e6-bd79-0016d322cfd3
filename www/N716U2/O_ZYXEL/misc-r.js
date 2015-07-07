

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Russian
tabArray=['Состояние','Конфигурация','Дополнительно','Перезагрузка','Заводские настройки (по умолчанию)','Обновление ПО'];

	//upgrade.htm
headArray[iIndex++] ="<br>Эта страница позволяет обновить встроенное ПО сервера печати.<br>Внимание: Перед загрузкой обновления, убедитесь, что вы правильно выбрали версию встроенного ПО. Если вы не знаете, какой файл встроенного ПО вам нужно использовать, обратитесь в службу технической поддержки местного поставщика оборудования.";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "Щелкните на кнопке <b>Заводские настройки</b>, а затем <b>OK</b> для загрузки фабричных настроек сервера печати. Предупреждение! Все текущие настройки будут стерты.";
textArray1[iIndex++] = "Щелкните на кнопке <b>Обновление ПО</b> для просмотра каталога обновлений встроенного ПО и перезагрузки сервера печати после установки нового встроенного ПО.";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;OK&nbsp;&nbsp;" onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="Обновление ПО";
textArray2[iIndex++]="Выбрать каталог и файл обновления встроенного ПО:";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="Обновление ПО" onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="Эта страница используется для перезагрузки сервера печати.<br>";
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>Перезагрузить сервер печати</B></FONT><br><br>Сохранить конфигурацию и перезагрузить сервер печати?<br><br>";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;OK&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

	//restart.htm
textArray4[iIndex++] = "Перезагрузка...";
textArray4[iIndex++] = "Подождите пока сервер перезагрузится.";
iIndex = 0;
	//urestart.htm
textArray5[iIndex++] = "Обновление выполнено успешно!";
textArray5[iIndex++] = "После обновления встроенного ПО сервер печати автоматически перезагрузится. Подождите.";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "Загрузка фабричной конфигурации ...";
textArray6[iIndex++] = "После загрузки фабричной конфигурации, сервер печати автоматически перезагрузится.<br><br>После перезагрузки откроется страница Status (Состояние).";
iIndex = 0;
// out of Russian
