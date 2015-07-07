
//vaiable
tabindex = 0;
textindex = 0;
var iIndex = 0;

//Language : Espanol
tabArray=['Estado','Configuración','Misc','Reiniciar','Sistema','TCP/IP','NetWare','AppleTalk','SNMP','SMB','',''];

//csystem
headArray[iIndex++] = "<BR>Esta página de configuración le permite ajustar la configuración general del sistema para el servidor de impresión.<br>";
//ctcpip.htm
headArray[iIndex++] = "<BR>Esta página de configuración le permite ajustar la configuración TCP/IP para el servidor de impresión.";
//cnetware.htm
headArray[iIndex++] = "<BR>Esta página de configuración le permite ajustar la función NetWare del servidor de impresión.";
//capple.htm 
headArray[iIndex++] ="<BR>Esta página de configuración le permite ajustar la configuración AppleTalk para el servidor de impresión.<br>";
//csnmp.htm
headArray[iIndex++] ="<BR>Esta página de configuración le permite ajustar la configuración SNMP para el servidor de impresión.";
//csmp.htm
headArray[iIndex++] ="<BR>Esta página muestra la configuración de compartir impresora para redes en Microsoft Windows.";
iIndex = 0;

// CSYSTEM.HTM
textArray0[iIndex++]="Configuración del sistema";
textArray0[iIndex++]="Nombre del servidor de impresión:";
textArray0[iIndex++]="Contacto del sistema:";
textArray0[iIndex++]="Ubicación del sistema:";
textArray0[iIndex++]="Contraseña del administrador";
textArray0[iIndex++]="Contraseña:";
textArray0[iIndex++]="Vuelva a escribir la contraseña:";
// Translate                                  only "Save & Restart" is to be translated
textArray0[iIndex++]='<input type="button"  value="Guardar y reiniciar" onClick="return CheckPwd(';
// Begin don't translate
textArray0[iIndex++]="'RESTART.HTM');";
textArray0[iIndex++]='">';
iIndex = 0;
// End don't translate
// CPRINTER.HTM
textArray1[iIndex++]="Puerto paralelo - Configuración bidireccional";
textArray1[iIndex++]="Puerto de impresora 1:";
textArray1[iIndex++]='<input type="button" value="Guardar y reiniciar" onClick="return SaveSetting(';
// Begin don't translate
textArray1[iIndex++]="'RESTART.HTM');";
textArray1[iIndex++]='">';
iIndex = 0;
// End don't translate
// CTCPIP.HTM
textArray2[iIndex++]="Configuración TCP/IP";
textArray2[iIndex++]="Obtener una configuración TCP/IP automáticamente (usar DHCP/BOOTP)";
textArray2[iIndex++]="Usar la siguiente configuración TCP/IP";
textArray2[iIndex++]="Dirección IP:";
textArray2[iIndex++]="Máscara de subred:";
textArray2[iIndex++]="Router predeterminado:";
// Translate                                  only "Save & Restart" is to be translated
textArray2[iIndex++]='<input type="button" value="Guardar y reiniciar" onClick="return SaveSetting(';
// Begin don't translate
textArray2[iIndex++]="'RESTART.HTM');";
textArray2[iIndex++]='">';
iIndex = 0;
// End don't translate
//CAPPLE.htm
textArray3[iIndex++]="Configuración AppleTalk";
textArray3[iIndex++]="Zona AppleTalk:";
textArray3[iIndex++]="Nombre del puerto:";
textArray3[iIndex++]="Configuración de impresión";
textArray3[iIndex++]="Tipo:";
textArray3[iIndex++]="Protocolo binario:";
// Translate                                  only "Save & Restart" is to be translated
textArray3[iIndex++]='<input type=button value="Guardar y reiniciar" onClick="return SaveSetting(';
// Begin don't translate
textArray3[iIndex++]="'RESTART.HTM');";
textArray3[iIndex++]='">';
// End don't translate

iIndex = 0;
// CSNMP.HTM
textArray4[iIndex++]="Configuración de comunidad SNMP";
textArray4[iIndex++]="Soporte para HP WebJetAdmin :";
textArray4[iIndex++]="Nombre de comunidad SNMP 1:";
textArray4[iIndex++]="Privilegios:";
textArray4[iIndex++]="Nombre de comunidad SNMP 2:";
textArray4[iIndex++]="Privilegios:";
textArray4[iIndex++]="Configuración de trampa SNMP";
textArray4[iIndex++]="Enviar trampas SNMP:";
textArray4[iIndex++]="Usar trampas de autenticación:";
textArray4[iIndex++]="Dirección de trampa 1:";
textArray4[iIndex++]="Dirección de trampa 2:";
// Translate                                  only "Save & Restart" is to be translated
textArray4[iIndex++]='<input type="button" value="Guardar y reiniciar" onClick="return SaveSetting(';
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
textArray7[iIndex++]="ERROR";
textArray7[iIndex++]="Volver";
iIndex = 0;

// CNETWARE.HTM
textArray8[iIndex++]="Configuración general";
textArray8[iIndex++]="Nombre del servidor de impresión:";
textArray8[iIndex++]="Tiempo de sondeo:";
textArray8[iIndex++]="&nbsp;segundos (mín: 3, máx: 29 segundos)";
textArray8[iIndex++]="Contraseña de acceso:";
textArray8[iIndex++]="Configuración NetWare NDS";
textArray8[iIndex++]="Usar modo NDS:";
textArray8[iIndex++]="Nombre del árbol NDS:";
textArray8[iIndex++]="Nombre del contexto NDS:";
textArray8[iIndex++]="Configuración de NetWare Bindery";
textArray8[iIndex++]="Usar modo Bindery:";
textArray8[iIndex++]="Nombre del servidor de archivos:";
textArray8[iIndex++]='<input type="button" value="Guardar y reiniciar" onClick="return SaveSetting(';
// Begin don't translate
textArray8[iIndex++]="'RESTART.HTM');";
textArray8[iIndex++]='">';
iIndex = 0;

// CSMB.HTM
textArray9[iIndex++]="Grupo de trabajo";
textArray9[iIndex++]="Nombre:";
textArray9[iIndex++]="Nombre compartido";
textArray9[iIndex++]="Impresora:";
textArray9[iIndex++]='<input type="button" value="Guardar y reiniciar" onClick="return SaveSetting(';
// Begin don't translate
textArray9[iIndex++]="'RESTART.HTM');";
textArray9[iIndex++]='">';
iIndex = 0;
// out of Espanol
