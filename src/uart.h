#ifndef __UART_H__
#define __UART_H__

#include <sys/stat.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <termios.h>
#include <libintl.h> // gettext 库支持
#include <locale.h> // 本地化locale的翻译支持
#include <vte-2.91/vte/vte.h>
#include <sys/stat.h>
#include <gdk/gdkkeysyms.h>

#define _(STRING)  gettext(STRING) 
#define PACKAGE    "uart" 
#define LOCALEDIR  "/usr/share/locale/" 

typedef struct 
{	
	GtkWidget *SelectPort;
	GtkWidget *SelectBaud;
	GtkWidget *SelectParity;
	GtkWidget *SelectStop;
	GtkWidget *SelectData;
    GtkWidget *CheckAutoSend;
    GtkWidget *CheckAutoClean;
    GtkWidget *CheckUseFile;
	GtkWidget *EntrySendCycle;
    GtkWidget *CheckReWriteFile;
    GtkWidget *CheckReTime; 
    GtkWidget *CheckHex;
	
}UartSetFace;

typedef struct 
{
	GtkWidget    *ReveTerminal;
	GtkWidget    *SendText;
	
}UartTextFace;

typedef struct
{
	GtkWidget *LabelState;
    GtkWidget *LabelRx;
    GtkWidget *LabelTx;
    GtkWidget *Button;
}UartBottomFace;

typedef struct
{
	char *UartPort;
	int   UartBaud;
	int   UartParity;
	int   UartData;
	int   UartStop;		
	int   fd;
	
}UartParamete;

typedef struct
{
	GtkWidget       *MainWindow;
	UartSetFace      ULC;
	UartTextFace     URC;
	UartBottomFace   UDC;
	UartParamete     UP;
    GSList          *Portlist;
    GtkListStore    *PortStore;
	int              Filefd;
	int              ShowHex;
	int              Redirect;
	int              ShowTime;
	int              ChooseFile;
    int              AutoCleanSendData;
    int              UseFile;
    int              UseFilefd;
    int              AutoSend;
    int              TimeId;
    int              SendCount;
    int              ReceCount;
    GtkWidget       *ChooseDialog;
	
}UartControl;

GMutex Mutex;
int CurrentData;
GCond Cond;
int Serialfd;
int ThreadEnd;
GtkWidget *MainWindow;
#endif
