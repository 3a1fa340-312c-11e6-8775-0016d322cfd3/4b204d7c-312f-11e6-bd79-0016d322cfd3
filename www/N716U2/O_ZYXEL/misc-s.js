

//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Espanol
tabArray=['Estado','Configuraci�n','Misc','Reiniciar','Valores predeterminados de f�brica','Actualizaci�n del firmware'];

	//upgrade.htm
headArray[iIndex++] ="<br>This page allows you to upgrade the firmware of the print server.<br>Note: please make sure the firmware is correct before you proceed. If you do not know which firmware file you should use, please contact your local dealer for technical support.";
iIndex = 0;

	//default.htm
textArray1[iIndex++] = "Haga clic en <b>Valores predeterminados de f�brica</b> y luego en <b>Aceptar</b> para volver a cargar la configuraci�n predeterminada en el servidor de impresi�n. �Aviso! Todas las configuraciones actuales se borrar�n.";
textArray1[iIndex++] = "Haga clic en <b>Actualizaci�n del firmware</b> para buscar el directorio de su firmware y volver a cargar el servidor de impresi�n con el nuevo firmware.";
// Translate                               Only OK is to be translated
textArray1[iIndex++] = '<input type=button  value="&nbsp;&nbsp;Aceptar&nbsp;&nbsp;" onClick="';
// Begin don't translate
textArray1[iIndex++] = "window.location='DRESTART.HTM'";
textArray1[iIndex++] = '">';
iIndex = 0;
// End don't translate

//upgrade.htm
textArray2[iIndex++]="Actualizaci�n del firmware";
textArray2[iIndex++]="Seleccione el directorio y archivo del firmware:";
// Begin don't translate
textArray2[iIndex++]='<input type=button value="Actualizaci�n del firmware" onClick="return WebUpgrade()">';
iIndex = 0;
// End don't translate

//reset.htm
textArray3[iIndex++]="Esta p�gina le permite reiniciar el servidor de impresi�n.<br>";
textArray3[iIndex++]="<FONT CLASS=F1 COLOR=#FF3300><B>Reiniciar el servidor de impresi�n</B></FONT><br><br>�Desea guardar la configuraci�n y reiniciar el servidor ahora?<br><br>";
// Translate                               Only OK is to be translated
textArray3[iIndex++]='<input type=button value="&nbsp;&nbsp;Aceptar&nbsp;&nbsp;" onClick="window.location=';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM'";
textArray3[iIndex++]='">';
iIndex = 0;
// End don't translate

	//restart.htm
textArray4[iIndex++] = "Reiniciando ...";
textArray4[iIndex++] = "Espere a que el servidor de impresi�n se reinicie.";
iIndex = 0;
	//urestart.htm
textArray5[iIndex++] = "�Actualizaci�n completada con �xito!";
textArray5[iIndex++] = "Tras actualizar el firmware, el servidor de impresi�n se reiniciar� autom�ticamente, espere un momento.";
iIndex = 0;
	//drestart.htm
textArray6[iIndex++] = "Cargando valores predeterminados de f�brica ...";
textArray6[iIndex++] = "Tras cargar la configuraci�n predeterminada, el servidor de impresi�n se reiniciar� autom�ticamente.<br><br>Volver� a la p�gina Estado cuando el servidor de impresi�n haya sido reiniciado.";
iIndex = 0;
// out of Espanol
