

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Espanol
tabArray=['Estado','Configuración','Misc','Reiniciar','Valores predeterminados de fábrica','Actualización del firmware'];

	//upgrade.htm
headArray[iIndex++] ="<br>This page allows you to upgrade the firmware of the print server.<br>Note: please make sure the firmware is correct before you proceed. If you do not know which firmware file you should use, please contact your local dealer for technical support.";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "Haga clic en <b>Valores predeterminados de fábrica</b> y luego en <b>Aceptar</b> para volver a cargar la configuración predeterminada en el servidor de impresión. ¡Aviso! Todas las configuraciones actuales se borrarán.";
textArray1[iIndex++] = "Haga clic en <b>Actualización del firmware</b> para buscar el directorio de su firmware y volver a cargar el servidor de impresión con el nuevo firmware.";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;Aceptar&nbsp;&nbsp;" onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="Actualización del firmware";
textArray2[iIndex++]="Seleccione el directorio y archivo del firmware:";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="Actualización del firmware" onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="Esta página le permite reiniciar el servidor de impresión.<br>";
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>Reiniciar el servidor de impresión</B></FONT><br><br>¿Desea guardar la configuración y reiniciar el servidor ahora?<br><br>";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;Aceptar&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

	//restart.htm
textArray4[iIndex++] = "Reiniciando ...";
textArray4[iIndex++] = "Espere a que el servidor de impresión se reinicie.";
iIndex = 0;
	//urestart.htm
textArray5[iIndex++] = "¡Actualización completada con éxito!";
textArray5[iIndex++] = "Tras actualizar el firmware, el servidor de impresión se reiniciará automáticamente, espere un momento.";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "Cargando valores predeterminados de fábrica ...";
textArray6[iIndex++] = "Tras cargar la configuración predeterminada, el servidor de impresión se reiniciará automáticamente.<br><br>Volverá a la página Estado cuando el servidor de impresión haya sido reiniciado.";
iIndex = 0;
// out of Espanol
