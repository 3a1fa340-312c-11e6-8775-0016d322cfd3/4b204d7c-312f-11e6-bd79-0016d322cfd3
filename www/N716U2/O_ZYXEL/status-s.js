//vaiable
tabindex = 0;
textindex = 0;

var iIndex = 0;

tabArray=['Estado','Configuración','Misc','Reiniciar','Sistema','Impresora','TCP/IP','NetWare','AppleTalk','SNMP','SMB',''];
//Language : Espanol

//system.htm
headArray[iIndex++] = "<BR>Esta página muestra la información general del sistema del servidor de impresión.<BR>";
//printer.htm
headArray[iIndex++] = "<BR>Esta página muestra información de la impresora conectada actualmente al servidor de impresión.<BR>Nota: Si su impresora no soporta la función bidireccional, puede que alguna información no aparezca correctamente.";
//tcpip.htm
headArray[iIndex++] = "<BR>Esta página muestra la configuración TCP/IP del servidor de impresión.<BR>";
//netware.htm
headArray[iIndex++] = "<BR>Esta página muestra la configuración NetWare del servidor de impresión.<BR>";
//apple.htm
headArray[iIndex++] = "<BR>Esta página muestra la configuración AppleTalk del servidor de impresión.<BR>";
//snmp.htm
headArray[iIndex++] = "<BR>Esta página muestra la configuración SNMP del servidor de impresión.<BR>";
//Smb.htm
headArray[iIndex++] = "<BR>Esta página muestra la configuración de compartir impresora para redes en Microsoft Windows.<BR>";
iIndex = 0;

// SYSTEM.HTM
textArray0[iIndex++]="Información del sistema";
textArray0[iIndex++]="Nombre del servidor de impresión:";
textArray0[iIndex++]="Contacto del sistema:";
textArray0[iIndex++]="Ubicación del sistema:";
textArray0[iIndex++]="Tiempo del sistema:";
textArray0[iIndex++]="Versión del firmware:";
textArray0[iIndex++]="Dirección MAC:";
iIndex = 0;

// PRINTER.HTM
textArray1[iIndex++]="Información de impresora";
textArray1[iIndex++]="Fabricante";
textArray1[iIndex++]="Número de modelo";
textArray1[iIndex++]="Idioma de impresión soportado";
textArray1[iIndex++]="Estado actual";
iIndex = 0;

// NETWARE.HTM
textArray2[iIndex++]="Configuración general";
textArray2[iIndex++]="Nombre del servidor de impresión:";
textArray2[iIndex++]="Tiempo de sondeo:";
textArray2[iIndex++]="Configuración NetWare NDS";
textArray2[iIndex++]="Usar modo NDS:";
textArray2[iIndex++]="Nombre del árbol NDS:";
textArray2[iIndex++]="Nombre del contexto NDS:";
textArray2[iIndex++]="Estado actual:";
textArray2[iIndex++]="Configuración de NetWare Bindery";
textArray2[iIndex++]="Usar modo Bindery:";
textArray2[iIndex++]="Nombre del servidor de archivos:";
textArray2[iIndex++]="Estado actual:";
iIndex = 0;

// TCPIP.HTM
textArray3[iIndex++]="Configuración TCP/IP";
textArray3[iIndex++]="Usar DHCP/BOOTP:";
textArray3[iIndex++]="Dirección IP:";
textArray3[iIndex++]="Máscara de subred:";
textArray3[iIndex++]="Puerta de enlace:";
iIndex = 0;

// APPLE.HTM
textArray4[iIndex++]="Configuración AppleTalk";
textArray4[iIndex++]="AppleTalk:";
textArray4[iIndex++]="Información de impresora";
textArray4[iIndex++]="Nombre del puerto:";
textArray4[iIndex++]="Tipo de impresora:";
textArray4[iIndex++]="Formato de datos:";
iIndex = 0;

// SNMP.HTM
textArray5[iIndex++]="Configuración de comunidad SNMP";
textArray5[iIndex++]="Comunidad SNMP 1:";
textArray5[iIndex++]="Comunidad SNMP 2:";
textArray5[iIndex++]="Configuración de trampa SNMP";
textArray5[iIndex++]="Enviar trampas SNMP:";
textArray5[iIndex++]="Usar trampas de autenticación:";
textArray5[iIndex++]="Dirección de trampa 1:";
textArray5[iIndex++]="Dirección de trampa 2:";
iIndex = 0;



// SMB.HTM
textArray7[iIndex++]="Grupo de trabajo";
textArray7[iIndex++]="Nombre:";
textArray7[iIndex++]="Nombre compartido";
textArray7[iIndex++]="Impresora:";


// out of Espanol
