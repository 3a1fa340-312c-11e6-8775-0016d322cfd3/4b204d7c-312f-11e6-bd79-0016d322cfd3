//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

tabArray=['Estado','Configuraci�n','Misc','Reiniciar','Sistema','Impresora','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
//Language : Espanol

//system.htm
headArray[iIndex++] = "<BR>Esta p�gina muestra la informaci�n general del sistema del servidor de impresi�n.<BR>";
//printer.htm
headArray[iIndex++] = "<BR>Esta p�gina muestra informaci�n de la impresora conectada actualmente al servidor de impresi�n.<BR>Nota: Si su impresora no soporta la funci�n bidireccional, puede que alguna informaci�n no aparezca correctamente.";
//tcpip.htm
headArray[iIndex++] = "<BR>Esta p�gina muestra la configuraci�n TCP/IP del servidor de impresi�n.<BR>";
//netware.htm
headArray[iIndex++] = "<BR>Esta p�gina muestra la configuraci�n NetWare del servidor de impresi�n.<BR>";
//apple.htm
headArray[iIndex++] = "<BR>Esta p�gina muestra la configuraci�n AppleTalk del servidor de impresi�n.<BR>";
//snmp.htm
headArray[iIndex++] = "<BR>Esta p�gina muestra la configuraci�n SNMP del servidor de impresi�n.<BR>";
//Smb.htm
headArray[iIndex++] = "<BR>Esta p�gina muestra la configuraci�n de compartir impresora para redes en Microsoft Windows.<BR>";
iIndex = 0;

// SYSTEM.HTM
textArray0[iIndex++]="Informaci�n del sistema";
textArray0[iIndex++]="Nombre del servidor de impresi�n:";
textArray0[iIndex++]="Contacto del sistema:";
textArray0[iIndex++]="Ubicaci�n del sistema:";
textArray0[iIndex++]="Tiempo del sistema:";
textArray0[iIndex++]="Versi�n del firmware:";
textArray0[iIndex++]="Direcci�n MAC:";
iIndex = 0;

// PRINTER.HTM
textArray1[iIndex++]="Informaci�n de impresora";
textArray1[iIndex++]="Fabricante";
textArray1[iIndex++]="N�mero de modelo";
textArray1[iIndex++]="Idioma de impresi�n soportado";
textArray1[iIndex++]="Estado actual";
iIndex = 0;

// NETWARE.HTM
textArray2[iIndex++]="Configuraci�n general";
textArray2[iIndex++]="Nombre del servidor de impresi�n:";
textArray2[iIndex++]="Tiempo de sondeo:";
textArray2[iIndex++]="Configuraci�n NetWare NDS";
textArray2[iIndex++]="Usar modo NDS:";
textArray2[iIndex++]="Nombre del �rbol NDS:";
textArray2[iIndex++]="Nombre del contexto NDS:";
textArray2[iIndex++]="Estado actual:";
textArray2[iIndex++]="Configuraci�n de NetWare Bindery";
textArray2[iIndex++]="Usar modo Bindery:";
textArray2[iIndex++]="Nombre del servidor de archivos:";
textArray2[iIndex++]="Estado actual:";
iIndex = 0;

// TCPIP.HTM
textArray3[iIndex++]="Configuraci�n TCP/IP";
textArray3[iIndex++]="Usar DHCP/BOOTP:";
textArray3[iIndex++]="Direcci�n IP:";
textArray3[iIndex++]="M�scara de subred:";
textArray3[iIndex++]="Puerta de enlace:";
iIndex = 0;

// APPLE.HTM
textArray4[iIndex++]="Configuraci�n AppleTalk";
textArray4[iIndex++]="AppleTalk:";
textArray4[iIndex++]="Informaci�n de impresora";
textArray4[iIndex++]="Nombre del puerto:";
textArray4[iIndex++]="Tipo de impresora:";
textArray4[iIndex++]="Formato de datos:";
iIndex = 0;

// SNMP.HTM
textArray5[iIndex++]="Configuraci�n de comunidad SNMP";
textArray5[iIndex++]="Comunidad SNMP 1:";
textArray5[iIndex++]="Comunidad SNMP 2:";
textArray5[iIndex++]="Configuraci�n de trampa SNMP";
textArray5[iIndex++]="Enviar trampas SNMP:";
textArray5[iIndex++]="Usar trampas de autenticaci�n:";
textArray5[iIndex++]="Direcci�n de trampa 1:";
textArray5[iIndex++]="Direcci�n de trampa 2:";
iIndex = 0;



// SMB.HTM
textArray7[iIndex++]="Grupo de trabajo";
textArray7[iIndex++]="Nombre:";
textArray7[iIndex++]="Nombre compartido";
textArray7[iIndex++]="Impresora:";


// out of Espanol
