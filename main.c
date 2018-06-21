#include <sys/stat.h>
#include <fcntl.h>
#include<gtk/gtk.h>
#include<glib.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include <time.h>
#include <math.h>
#include <termios.h>
#include <vte-2.91/vte/vte.h>
#include <sys/stat.h>
#include<gdk/gdkkeysyms.h>

#define     OPEN       1
#define     CLOSE      0
#define     TYPEMSG    "<span foreground='red'font_desc='13'>%s </span>"
#define     ERROR      1
#define     WARING     2
#define     INFOR      3
#define     QUESTION   4
#define     SAVE       0
#define     CHOOSE     1


typedef struct 
{	
	GtkWidget *SelectPort;
	GtkWidget *SelectBaud;
	GtkWidget *SelectParity;
	GtkWidget *SelectStop;
	GtkWidget *SelectData;
    GtkWidget *CheckAutoSend;
    GtkWidget *CheckUseFile;
	GtkWidget *EntrySendCycle;
	
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
	char UartPort[30];
	int  UartBaud;
	int  UartParity;
	int  UartData;
	int  UartStop;		
	int  fd;
	
}UartParamete;

typedef struct
{
	GtkWidget       *MainWindow;
	UartSetFace      ULC;
	UartTextFace     URC;
	UartBottomFace   UDC;
	UartParamete     UP;
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
    GtkWidget       *ChooseDialog;
	
}UartControl;

int gnSwitchSatue;
GMutex Mutex;
int CurrentData;
GCond Cond;
int Serialfd;
int ThreadEnd;
GtkWidget *MainWindow;
void CleanSendDate(UartControl *uc);
int WriteUart(const char *WriteData,int WriteLen,UartControl *uc);
int MessageReport(const char *Title,const char *Msg,int nType)
{
    GtkWidget *dialog;
    int nRet;
    switch(nType)
    {
        case ERROR:
        {
            dialog = gtk_message_dialog_new(GTK_WINDOW(MainWindow),
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_MESSAGE_ERROR,
                                            GTK_BUTTONS_OK,
                                            Title);
            break;
        }
        case WARING:
        {
            dialog = gtk_message_dialog_new(GTK_WINDOW(MainWindow),
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_MESSAGE_WARNING,
                                            GTK_BUTTONS_OK,
                                            Title);
            break;
        }
        case INFOR:
        {
            dialog = gtk_message_dialog_new(GTK_WINDOW(MainWindow),
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_MESSAGE_INFO,
                                            GTK_BUTTONS_OK,
                                            Title);
            break;
        }
        case QUESTION:
        {
            dialog = gtk_message_dialog_new(GTK_WINDOW(MainWindow),
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_MESSAGE_QUESTION,
                                            GTK_BUTTONS_YES_NO,
                                            Title);
            break;
        }
        default :
            break;

    }
    gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog),TYPEMSG,Msg);
    gtk_window_set_title(GTK_WINDOW(dialog),("Message"));
    nRet =  gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return nRet;
}

void SetWidgetStyle(GtkWidget * Widget,const char *Color,int FontSize)
{
    GdkRGBA Rgb;
    PangoFontDescription *font_desc;

    if(gdk_rgba_parse(&Rgb,Color) == TRUE)
        gtk_widget_override_color(Widget, GTK_STATE_NORMAL, &Rgb);

    font_desc = pango_font_description_from_string("Sans Blod 20");  
    pango_font_description_set_size (font_desc, FontSize * PANGO_SCALE);
    gtk_widget_modify_font (Widget, font_desc);
    pango_font_description_free (font_desc);
}
void SetLableFontType(GtkWidget *Lable ,const char *Color,int FontSzie,const char *Word)
{
    char LableTypeBuf[200] = { 0 };

    sprintf(LableTypeBuf,"<span foreground=\'%s\'weight=\'light\'font_desc=\'%d\'>%s</span>",
                             Color,FontSzie,Word);
    gtk_label_set_markup(GTK_LABEL(Lable),LableTypeBuf); 

} 
 
GtkWidget *CreateMainWindow(void)
{
	GtkWidget *WindowMain;
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    
    WindowMain = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect_swapped(G_OBJECT(WindowMain), "destroy", G_CALLBACK(gtk_main_quit), NULL);  
    gtk_window_set_title(GTK_WINDOW(WindowMain),"Serial port assistant");
	gtk_window_set_position(GTK_WINDOW(WindowMain), GTK_WIN_POS_CENTER);
	gtk_widget_realize(WindowMain);
    gtk_container_set_border_width(GTK_CONTAINER(WindowMain),10);
	gtk_widget_set_size_request(WindowMain, 500, 500);
    
	return WindowMain;
}	

GtkWidget *SetComboBox(GtkListStore  *Store)
{
	
	GtkCellRenderer *Renderer;
    GtkWidget *ComboUser;
    
    ComboUser = gtk_combo_box_new_with_model(GTK_TREE_MODEL(Store));
    g_object_unref(G_OBJECT(Store));
    Renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(ComboUser),Renderer,TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(ComboUser),Renderer,"text",0,NULL);
    gtk_combo_box_set_active(GTK_COMBO_BOX(ComboUser),1);

    return ComboUser;
	
}
GtkWidget *SetComPort(void)
{
    GtkListStore    *Store;
    GtkTreeIter     Iter;
    

    Store = gtk_list_store_new(1,G_TYPE_STRING);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"ttyS0",-1);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"ttyUSB0",-1);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"ttyS2",-1);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"ttyS3",-1);

    return SetComboBox(Store);
}
GtkWidget *SetBaud(void)
{
    GtkListStore    *Store;
    GtkTreeIter     Iter;
	int i = 0;
	double Baud;
	char StrBaud[20] = { 0 };

    Store = gtk_list_store_new(1,G_TYPE_STRING);
    
    for( i = 0; i < 8; i++)
	{
		Baud = ldexp(300, i);
		sprintf(StrBaud,"%d",(int)Baud);
    	gtk_list_store_append(Store,&Iter);
    	gtk_list_store_set(Store,&Iter,0,StrBaud,-1);
    	memset(StrBaud,'\0',strlen(StrBaud));
    }
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"56000",-1);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"115200",-1);

 	return SetComboBox(Store);
  
}

GtkWidget *SetParity(void)
{
    GtkListStore    *Store;
    GtkTreeIter     Iter;

    Store = gtk_list_store_new(1,G_TYPE_STRING);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"NONE",-1);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"ODD",-1);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"EVEN",-1);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"MARK",-1);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"SPACE",-1);

    return SetComboBox(Store);

}
GtkWidget *SetStop(void)
{
    GtkListStore    *Store;
    GtkTreeIter     Iter;
   

    Store = gtk_list_store_new(1,G_TYPE_STRING);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"1",-1);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"1.5",-1);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"2",-1);

  	return SetComboBox(Store);
}
GtkWidget *SetData(void)
{
    GtkListStore    *Store;
    GtkTreeIter     Iter;
    
    Store = gtk_list_store_new(1,G_TYPE_STRING);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"8",-1);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"7",-1);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"6",-1);
    gtk_list_store_append(Store,&Iter);
    gtk_list_store_set(Store,&Iter,0,"5",-1);


    return SetComboBox(Store);
}

int GetSwitchState( const char *ButtonName)
{
    if(strstr(ButtonName,"Close") != NULL)
        return 0;
    else
        return 1;

}        
int InitSerial(UartControl *uc)
{
	int fd;
	struct termios termios_old, termios_new;
	
	fd = open(uc->UP.UartPort, O_RDWR | O_NOCTTY | O_NDELAY);
	if(fd < 0)  
	{			
		return -1;
	}
	
	if(fcntl(fd, F_SETFL, 0) < 0)		//擦除以前的标志位，恢复成默认设置
	{
		return -1;	
	}
		
	if(isatty(STDIN_FILENO) == 0)		//判断是否为终端设备，若为终端设备值为1，否则执行下面的打印
	{
		return -1;
	}		
	

	if(tcgetattr(fd, &termios_old) != 0)
	{
		return -1;
	}
	
	bzero(&termios_new, sizeof(termios_new));

	termios_new.c_cflag |= CLOCAL;
	termios_new.c_cflag |= CREAD;
	termios_new.c_cflag &= ~CSIZE;
	
	switch(uc->UP.UartData)
	{
		case 5:
			termios_new.c_cflag |= CS5;
			break;
		case 6:
			termios_new.c_cflag |= CS6;
			break;	
		case 7:
			termios_new.c_cflag |= CS7;
			break;
		case 8:
			termios_new.c_cflag |= CS8;
			break;
		default:
			termios_new.c_cflag |= CS8;
			break;
	}

	switch(uc->UP.UartParity)
	{
		case 0:		//no parity check
			termios_new.c_cflag &= ~PARENB;
			break;
		case 1:		//odd check
			termios_new.c_cflag |= PARENB;
			termios_new.c_cflag |= PARODD;
			termios_new.c_cflag |= (INPCK | ISTRIP);
			break;
		case 2:		//even check
			termios_new.c_cflag |= (INPCK | ISTRIP);
			termios_new.c_cflag |= PARENB;
			termios_new.c_cflag &= ~PARODD;
			break;
		case 3:
			termios_new.c_cflag |= PARENB | CMSPAR |PARODD;
			break;
		case 4:
			termios_new.c_cflag |= PARENB | CS8 | CMSPAR;
		default:			//no parity check
			termios_new.c_cflag &= ~PARENB;
			break;    
	}
	
	switch(uc->UP.UartBaud)
	{
		case 1200:
			cfsetispeed(&termios_new, B1200);
			cfsetospeed(&termios_new, B1200);
			break;
		case 2400:
			cfsetispeed(&termios_new, B2400);
			cfsetospeed(&termios_new, B2400);
			break;
		case 4800:
			cfsetispeed(&termios_new, B4800);
			cfsetospeed(&termios_new, B4800);
			break;	
		case 9600:
			cfsetispeed(&termios_new, B9600);
			cfsetospeed(&termios_new, B9600);
			break;
		case 19200:
			cfsetispeed(&termios_new, B19200);
			cfsetospeed(&termios_new, B19200);
			break;
		case 38400:
			cfsetispeed(&termios_new, B38400);
			cfsetospeed(&termios_new, B38400);
			break;
		case 57600:
			cfsetispeed(&termios_new, B57600);
			cfsetospeed(&termios_new, B57600);
			break;
		case 115200:
			cfsetispeed(&termios_new, B115200);
			cfsetospeed(&termios_new, B115200);
			break;
		case 230400:
			cfsetispeed(&termios_new, B230400);
			cfsetospeed(&termios_new, B230400);
			break;
		default:
			cfsetispeed(&termios_new, B9600);
			cfsetospeed(&termios_new, B9600);
			break;
	}
	
	if(uc->UP.UartStop == 1)
		termios_new.c_cflag &= ~CSTOPB;
	else if(uc->UP.UartStop == 2)
		termios_new.c_cflag |= CSTOPB;
	else
		termios_new.c_cflag &= ~CSTOPB;

	termios_new.c_cc[VMIN]  = 0;		
	termios_new.c_cc[VTIME] = 0;		
	tcflush(fd, TCIFLUSH);				
	if(tcsetattr(fd, TCSANOW, &termios_new) != 0)
	{
		
		return -1;
	}
	
	return fd;
}	
void LockSetSerial(UartControl *uc)
{
	gtk_widget_set_sensitive(uc->ULC.SelectPort, FALSE);
	gtk_widget_set_sensitive(uc->ULC.SelectBaud, FALSE);
	gtk_widget_set_sensitive(uc->ULC.SelectParity, FALSE);
	gtk_widget_set_sensitive(uc->ULC.SelectStop, FALSE);
	gtk_widget_set_sensitive(uc->ULC.SelectData, FALSE);
	
}     	

void UnLockSetSerial(UartControl *uc)
{
	gtk_widget_set_sensitive(uc->ULC.SelectPort, TRUE);
	gtk_widget_set_sensitive(uc->ULC.SelectBaud, TRUE);
	gtk_widget_set_sensitive(uc->ULC.SelectParity, TRUE);
	gtk_widget_set_sensitive(uc->ULC.SelectStop, TRUE);
	gtk_widget_set_sensitive(uc->ULC.SelectData, TRUE);
	
}   

void AddReceiveTime(UartControl *uc)
{
	struct tm *t;
    time_t tt;
    time(&tt);
    t = localtime(&tt);
    char TimeBuf[128] = { 0 };
    sprintf(TimeBuf,"%4d-%02d-%02d  %02d:%02d:%02d\r\n", t->tm_year + 1900, 
    													 t->tm_mon + 1, 
    													 t->tm_mday, 
    													 t->tm_hour, 
    													 t->tm_min, 
    													 t->tm_sec);
	vte_terminal_feed(VTE_TERMINAL(uc->URC.ReveTerminal), TimeBuf, strlen(TimeBuf));

}    
static int Frist;
static int AddTime;
int ConversionHex(char *ReadBuf,int size,UartControl *uc)
{
    static gchar data_byte[6];
    gint i = 0;
    if(size == 0)
    return -1;
    if(Frist == 0)
    {
        AddTime = 1;
        Frist = 1;
    } 
    if(AddTime == 1 && uc->ShowTime == 1)
    {
        AddReceiveTime(uc);	
        AddTime = 0;
    }        
    while(i < size) 
    {
        
        sprintf(data_byte, "%02X ", (guchar)ReadBuf[i]);
        i++;
        vte_terminal_feed(VTE_TERMINAL(uc->URC.ReveTerminal), data_byte, 3);
        if((guchar)ReadBuf[i-1] == 0x0A)
    	{    
            AddTime = 1;
            vte_terminal_feed(VTE_TERMINAL(uc->URC.ReveTerminal),"\r\n", -1);
        }
    }

}    
int AlignData(char *ReadBuf,int size,UartControl *uc)
{
    int pos;
    GString *buffer_tmp;
    gchar *in_buffer;

    buffer_tmp =  g_string_new(ReadBuf);
    in_buffer=buffer_tmp->str;
    in_buffer += size;
    if(Frist == 0 && size > 0)
    {
        AddTime = 1; 
        Frist = 1;
    }        
    if(uc->ShowTime == 1 && AddTime == 1 && size > 0)
    {
    	AddReceiveTime(uc);	
    	AddTime = 0;
    }	
    for (pos = size; pos > 0; pos--) {
        in_buffer--;
        if(*in_buffer=='\r' && *(in_buffer+1) != '\n'){
            g_string_insert_c(buffer_tmp, pos, '\n');
            size += 1;
        }
        if(*in_buffer=='\n' && *(in_buffer-1) != '\r'){
            g_string_insert_c(buffer_tmp, pos-1, '\r');
            AddTime = 1;
            size += 1;
        }
        if(*in_buffer=='\n' && *(in_buffer-1) == '\r')
        {
            AddTime = 1;
        }        
    }
    vte_terminal_feed(VTE_TERMINAL(uc->URC.ReveTerminal), buffer_tmp->str, size);

}     
int WriteDataToFile(char *ReadBuf,int size,UartControl *uc)
{
	int pos;
    GString *buffer_tmp;
    gchar *in_buffer;

    buffer_tmp =  g_string_new(ReadBuf);
    in_buffer=buffer_tmp->str;
    in_buffer += size;

    for (pos = size; pos > 0; pos--) {
        in_buffer--;
        if(*in_buffer=='\r' && *(in_buffer+1) != '\n'){
            g_string_insert_c(buffer_tmp, pos, '\n');
            size += 1;
        }
        if(*in_buffer=='\n' && *(in_buffer-1) != '\r'){
            g_string_insert_c(buffer_tmp, pos-1, '\r');
            size += 1;
        }
    }
	write(uc->Filefd,buffer_tmp->str, size);
}  
int DataHandle(char *ReadBuf,UartControl *uc)
{
	if(uc->Redirect == 1 && uc->Filefd > 0)
	{
		WriteDataToFile(ReadBuf,strlen(ReadBuf),uc);
	}	
	else
	{
        if(uc->ShowHex == 1)
            ConversionHex(ReadBuf,strlen(ReadBuf),uc);
        else
            AlignData(ReadBuf,strlen(ReadBuf),uc);			
	}		
}    
gpointer ReadUart(gpointer data)
{
	UartControl *uc = (UartControl *) data;
	int ReadLen;
	fd_set rd;
    unsigned char ReadBuf[1024] = {0};
    struct timeval Timeout={0,0}; 
    while(1)
    {
    	FD_ZERO(&rd);
  		FD_SET(Serialfd,&rd);
  		while(FD_ISSET(Serialfd,&rd))
  		{
    	    if(ThreadEnd == 1)
                g_thread_exit(NULL);    
            if(select(Serialfd+1,&rd,NULL,NULL,&Timeout) < 0)
      			MessageReport(("Read Serial"),("Read Serial Fail"),ERROR);
    		else
    		{
                usleep(200000);
      			g_mutex_lock(&Mutex);
				ReadLen = read(Serialfd, ReadBuf, 1024);
				g_cond_signal (&Cond);
                CurrentData = 1;
				g_mutex_unlock(&Mutex);
				DataHandle(ReadBuf,uc);
                memset(ReadBuf,'\0',strlen(ReadBuf));
      		}
    	}
  	}
		
}
gpointer CreateReadUart(UartControl *uc)
{
	return g_thread_new("ReadUart",(GThreadFunc)ReadUart,(gpointer)uc);
}
static void OpenSerial(GtkWidget *Button,gpointer user_data)
{
    int SwitchState;
    int OpenState;
	UartControl *uc = (UartControl *) user_data;
	
    SwitchState = GetSwitchState(gtk_button_get_label(GTK_BUTTON(Button)));
    if(SwitchState == CLOSE)
    {
        if(Serialfd < 0)
        {
        	MessageReport(("close Serial"),("close Serial Fail"),ERROR);
        }	
        else
        {
            ThreadEnd = 1;
        	UnLockSetSerial(uc); 
            close(Serialfd);
        	Serialfd = -1;
            uc->UP.fd = -1;
            if(uc->Redirect == 1 && uc->Filefd > 0)
            {
                close(uc->Filefd);
                uc->Filefd = -1;
            }        
        	gtk_button_set_label(GTK_BUTTON(Button),"● Open");
        	SetWidgetStyle(Button,"black",13);
        }	
    }
    else
    {        
        OpenState = InitSerial(uc);
        if(OpenState < 0)
        	MessageReport(("Open Serial"),("Open Serial Fail"),ERROR);
        else
        {	ThreadEnd = 0;
   			LockSetSerial(uc);    
   			uc->UP.fd =  OpenState;	
   			Serialfd = OpenState;
        	gtk_button_set_label(GTK_BUTTON(Button),"● Close");
        	SetWidgetStyle(Button,"red",13);
        	CreateReadUart(uc);
   
		}
    }    
    printf("port = %s baud = %d parity = %d data = %d stop = %d\r\n",uc->UP.UartPort,uc->UP.UartBaud,uc->UP.UartParity,uc->UP.UartData,uc->UP.UartStop);

}  
  
static void SwitchUartPort(GtkWidget *widget,gpointer data)
{
	char *text;
    GtkTreeIter   iter;
    GtkTreeModel *model;
    char Path[30] = { 0 };
    
    UartControl *uc = (UartControl *) data;

	if( gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter ) )
    {
    	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
        gtk_tree_model_get( model, &iter, 0, &text, -1 );
    }
   	memset(uc->UP.UartPort,'\0',strlen(uc->UP.UartPort));
	sprintf(Path,"/dev/%s",text);
    memcpy(uc->UP.UartPort,Path,strlen(Path));
	g_free(text);
     
} 
    
static void SwitchUartBaud(GtkWidget *widget,gpointer data)
{
	char *text;
    GtkTreeIter   iter;
    GtkTreeModel *model;
    
    UartControl *uc = (UartControl *) data;

	if( gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter ) )
    {
    	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
        gtk_tree_model_get( model, &iter, 0, &text, -1 );
    }
    
    uc->UP.UartBaud = atoi(text);
	g_free(text);
     
} 

static void SwitchUartParity(GtkWidget *widget,gpointer data)
{
	char *text;
    GtkTreeIter   iter;
    GtkTreeModel *model;
    
    UartControl *uc = (UartControl *) data;

	if( gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter ) )
    {
    	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
        gtk_tree_model_get( model, &iter, 0, &text, -1 );
    }
    
   	if(strcmp(text,"NONE") == 0)
   		uc->UP.UartParity = 0;
   	else if(strcmp(text,"EVEN") == 0)
   		uc->UP.UartParity = 2;
   	else if(strcmp(text,"ODD") == 0)
   		uc->UP.UartParity = 1;
   	else if(strcmp(text,"MARK") == 0)
   		uc->UP.UartParity = 3;
   	else if(strcmp(text,"SPACE") == 0)
   		uc->UP.UartParity = 4;
   	
	g_free(text);
     
} 
static void SwitchUartData(GtkWidget *widget,gpointer data)
{
	char *text;
    GtkTreeIter   iter;
    GtkTreeModel *model;
    
    UartControl *uc = (UartControl *) data;

	if( gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter ) )
    {
    	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
        gtk_tree_model_get( model, &iter, 0, &text, -1 );
    }
    printf("text = %s\r\n",text);
    uc->UP.UartData = atoi(text);
	g_free(text);
     
} 
static void SwitchUartStop(GtkWidget *widget,gpointer data)
{
	char *text;
    GtkTreeIter   iter;
    GtkTreeModel *model;
    
    UartControl *uc = (UartControl *) data;

	if( gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter ) )
    {
    	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
        gtk_tree_model_get( model, &iter, 0, &text, -1 );
    }
   
    uc->UP.UartStop = atoi(text);
	g_free(text);
     
}   
void SerialPortSetup(GtkWidget *Hbox,UartControl *uc)
{
	GtkWidget *Vpaned;
	GtkWidget *Frame;
	GtkWidget *Table;
	GtkWidget *LableComPort;
	GtkWidget *SelectPort;
	GtkWidget *LableBaud;
	GtkWidget *SelectBaud;
	GtkWidget *LabelParity;
	GtkWidget *SelectParity;
	GtkWidget *LabelStop;
	GtkWidget *SelectStop;
	GtkWidget *LabelData;
	GtkWidget *SelectData;
	GtkWidget *Hseparator;
	GtkWidget *Button;

	Vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
	gtk_container_add (GTK_CONTAINER (Hbox), Vpaned);
	
    Frame = gtk_frame_new ("Serial Setting");
    SetWidgetStyle(Frame,"black",12);
	gtk_frame_set_label_align(GTK_FRAME(Frame),0.5,0.3);
	gtk_paned_pack1 (GTK_PANED (Vpaned), Frame, FALSE, TRUE);
	
    Table = gtk_grid_new();
    gtk_container_add (GTK_CONTAINER (Vpaned), Table);
	gtk_grid_set_column_homogeneous(GTK_GRID(Table),TRUE);

    LableComPort = gtk_label_new(NULL);
    SetLableFontType(LableComPort,"black",10,("Serial Number"));
    gtk_grid_attach(GTK_GRID(Table) , LableComPort , 0 , 0 , 1 , 1);
   	
    SelectPort = SetComPort();
    gtk_combo_box_set_active(GTK_COMBO_BOX(SelectPort),0);
    uc->ULC.SelectPort = SelectPort;
    gtk_grid_attach(GTK_GRID(Table) ,SelectPort , 1 , 0 , 1 , 1);
    g_signal_connect(G_OBJECT(SelectPort),"changed",G_CALLBACK(SwitchUartPort),(gpointer) uc);

	LableBaud = gtk_label_new(NULL);
    SetLableFontType(LableBaud,"black",10,("Baud Rate"));
    gtk_grid_attach(GTK_GRID(Table) , LableBaud , 0 , 1 , 1 , 1);
	
   	SelectBaud = SetBaud();
    gtk_combo_box_set_active(GTK_COMBO_BOX(SelectBaud),5);
    uc->ULC.SelectBaud = SelectBaud;
    gtk_grid_attach(GTK_GRID(Table) ,SelectBaud , 1 , 1 , 1 , 1);
    g_signal_connect(G_OBJECT(SelectBaud),"changed",G_CALLBACK(SwitchUartBaud),(gpointer) uc);
    
	LabelParity = gtk_label_new(NULL);
    SetLableFontType(LabelParity,"black",10,("Parity Bit"));
    gtk_grid_attach(GTK_GRID(Table) , LabelParity , 0 , 2 , 1 , 1);
	
   	SelectParity = SetParity();
    gtk_combo_box_set_active(GTK_COMBO_BOX(SelectParity),0);
    uc->ULC.SelectParity = SelectParity;
    gtk_grid_attach(GTK_GRID(Table) ,SelectParity , 1 , 2 , 1 , 1);
	g_signal_connect(G_OBJECT(SelectParity),"changed",G_CALLBACK(SwitchUartParity),(gpointer) uc);

	LabelData = gtk_label_new(NULL);
    SetLableFontType(LabelData,"black",10,("Data Bit"));
    gtk_grid_attach(GTK_GRID(Table) , LabelData , 0 , 3 , 1 , 1);
	
   	SelectData = SetData();
    gtk_combo_box_set_active(GTK_COMBO_BOX(SelectData),0);
    gtk_grid_attach(GTK_GRID(Table) ,SelectData , 1 , 3 , 1 , 1);
    uc->ULC.SelectData = SelectData;
	g_signal_connect(G_OBJECT(SelectData),"changed",G_CALLBACK(SwitchUartData),(gpointer) uc);
	
	LabelStop = gtk_label_new(NULL);
    SetLableFontType(LabelStop,"black",10,("Stop Bit"));
    gtk_grid_attach(GTK_GRID(Table) , LabelStop , 0 , 4 , 1 , 1);
	
   	SelectStop = SetStop();
    gtk_combo_box_set_active(GTK_COMBO_BOX(SelectStop),0);
    uc->ULC.SelectStop = SelectStop;
    gtk_grid_attach(GTK_GRID(Table) ,SelectStop , 1 , 4 , 1 , 1);
	g_signal_connect(G_OBJECT(SelectStop),"changed",G_CALLBACK(SwitchUartStop),(gpointer) uc);
   
    Button = gtk_button_new_with_label ("● open");
    SetWidgetStyle(Button,"black",13);
    gtk_grid_attach(GTK_GRID(Table) , Button , 0 , 5 , 2 , 1);
    g_signal_connect(G_OBJECT(Button), "clicked", G_CALLBACK(OpenSerial), (gpointer) uc);  
     
	Hseparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(Table) , Hseparator , 0 , 7 , 2 , 1);
    
    gtk_grid_set_row_spacing(GTK_GRID(Table), 1);
    gtk_grid_set_column_spacing(GTK_GRID(Table), 1);
}
gboolean
user_function (GtkLabel *label,
               gchar    *uri,
               gpointer  user_data)
{
	printf("wwwwwwwwwww\r\n");	
	
	UartControl *uc = (UartControl *) user_data;
	
}   
void ClearTerminalData (GtkLabel *label,
               			gchar    *uri,
               			gpointer  data)
{
	UartControl *uc = (UartControl *) data;
    if(uc->URC.ReveTerminal)
        vte_terminal_reset(VTE_TERMINAL(uc->URC.ReveTerminal), TRUE, TRUE);
	
}     
int OpenFileName  (const char * FileName)
{
	int fd = -1;
	int Ret;
	
	if(access(FileName,F_OK) == -1 )
	{
		fd = open(FileName,O_RDWR|O_APPEND|O_CREAT,0777);
		if(fd < 0)
		{
			MessageReport(("Create File"),("Create File Fail"),ERROR);
		}	
	}
	else if(access(FileName, W_OK) == -1)
	{
		MessageReport(("Open File"),("Open File Fail"),ERROR);	
	}	
    else
	{
		Ret = MessageReport(("Open File"),("Whether to clear the contents of the file"),QUESTION);	
		if(Ret == GTK_RESPONSE_YES)
    	{
        	fd = open(FileName,O_WRONLY|O_APPEND|O_TRUNC);	
        	if(fd < 0)
        	{
        		MessageReport(("Create File"),("Create File Fail"),ERROR);
        	}	
    	}
    	else 
    	{
        	fd = open(FileName,O_WRONLY|O_APPEND);	
        	if(fd < 0)
        	{
        		MessageReport(("Create File"),("Create File Fail"),ERROR);
        	}	
    	}
	}			
		
	return fd;
}        			 
int OpenUseFile (const char *FileName)
{
    int fd = -1;
    
    if(access(FileName,R_OK) == -1)
	{
		MessageReport(("Open File"),("Open File Fail"),ERROR);	
        return -1;
	}

    fd = open(FileName,O_RDONLY);	
    if(fd < 0)
    {
        MessageReport(("Create File"),("Create File Fail"),ERROR);
    }	
    return fd;    
}        
void ChooseFileWrite (GtkButton *button, gpointer data)
{
    char *FileName = NULL;
    int fd;
    UartControl *uc = (UartControl *)data;

    FileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (uc->ChooseDialog));
   
    switch (uc->ChooseFile ){
        case SAVE:
           	fd = OpenFileName(FileName);
           	if(fd > 0 )
           	{	
           		uc->Filefd = fd;
           	}
            break;
        case CHOOSE:
            fd = OpenUseFile(FileName);
            if(fd > 0)
            {
                uc->UseFilefd = fd;
            }        
            break;
        default:
            break;

    }
    gtk_widget_destroy(GTK_WIDGET(uc->ChooseDialog));
}
void  CloseChooseFileWrite(GtkButton *button, gpointer data)
{
	UartControl *uc = (UartControl *)data;
    gtk_widget_destroy(GTK_WIDGET(uc->ChooseDialog));
}

GtkWidget* CreateFileChoose (UartControl *uc)
{
    GtkWidget *dialog_vbox;
    GtkWidget *dialog_action_area;
    GtkWidget *ButtonCancel;
    GtkWidget *ButtonOk;
	GtkWidget *FileChoose;
	
    if(uc->ChooseFile == SAVE) 
    {
        FileChoose = gtk_file_chooser_dialog_new ("Choose Write File", NULL, GTK_FILE_CHOOSER_ACTION_SAVE, NULL);
        ButtonOk = gtk_button_new_with_label ("save");
    }
    else 
    {
    	FileChoose = gtk_file_chooser_dialog_new ("", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, NULL);
        ButtonOk = gtk_button_new_with_label ("Choose");	
    }
    
    uc->ChooseDialog = FileChoose;
    gtk_window_set_type_hint (GTK_WINDOW (FileChoose), GDK_WINDOW_TYPE_HINT_DIALOG);

    ButtonCancel = gtk_button_new_with_label ("Cancel");
    gtk_widget_show (ButtonCancel);
    gtk_dialog_add_action_widget (GTK_DIALOG (FileChoose), ButtonCancel, GTK_RESPONSE_CANCEL);
  
    gtk_widget_show (ButtonOk);
    gtk_dialog_add_action_widget (GTK_DIALOG (FileChoose), ButtonOk, GTK_RESPONSE_OK);
   
    g_signal_connect ((gpointer) ButtonCancel, "clicked",
            G_CALLBACK (CloseChooseFileWrite),uc);
    g_signal_connect ((gpointer) ButtonOk, "clicked",
            G_CALLBACK (ChooseFileWrite),uc);
    //gtk_widget_grab_default (ButtonOk);

    return FileChoose;
}

int LoadFile (UartControl *uc)
{
	
    GtkWidget *WindowFileChoose;
     
    WindowFileChoose = (GtkWidget *)CreateFileChoose(uc);
    gtk_widget_show (WindowFileChoose);
}
void SwitchWriteFile(GtkWidget *Check,gpointer  data)
{
	UartControl *uc = (UartControl *) data;	
	if(uc->Redirect == 1)
		uc->Redirect = 0;
	else
	{	
		uc->ChooseFile = SAVE; 
		LoadFile(uc);
        uc->Redirect = 1;
	}	
}  
void SwitchReceiveTime(GtkWidget *Check,gpointer  data)
{
	UartControl *uc = (UartControl *) data;	
	if(uc->ShowTime == 0)
		uc->ShowTime = 1;
	else
		uc->ShowTime = 0;
} 

void SwitchReceiveHex(GtkWidget *Check,gpointer  data)
{
	UartControl *uc = (UartControl *) data;
	if(uc->ShowHex == 0)
		uc->ShowHex = 1;
	else
		uc->ShowHex = 0;		
} 
  
void ReceiveSet(GtkWidget *Hbox,UartControl *uc)
{
	GtkWidget *CheckReWriteFile;
	GtkWidget *CheckReTime;
	GtkWidget *CheckHex;
	GtkWidget *CheckStop;
	GtkWidget *Hseparator;
	GtkWidget *Vpaned;
	GtkWidget *RcvFrame;
    GtkWidget *LabelSave;
    GtkWidget *LabelClear;
	GtkWidget *LabelSpace;
	GtkWidget *Table;
	
	Vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
	gtk_container_add (GTK_CONTAINER (Hbox), Vpaned);
	RcvFrame = gtk_frame_new ("Receive Setting");
	gtk_frame_set_label_align(GTK_FRAME(RcvFrame),0.5,0.3);
    SetWidgetStyle(RcvFrame,"black",12);
	gtk_paned_pack1 (GTK_PANED (Vpaned), RcvFrame, FALSE, TRUE);
	
	Table = gtk_grid_new();
    gtk_container_add (GTK_CONTAINER (Vpaned), Table);
	gtk_grid_set_column_homogeneous(GTK_GRID(Table),TRUE);

	LabelSpace = gtk_label_new(" ");
	gtk_grid_attach(GTK_GRID(Table) ,LabelSpace , 0 , 0 , 2 , 1);
	
	CheckReWriteFile = gtk_check_button_new_with_label("Receive Write File");
	gtk_grid_attach(GTK_GRID(Table) , CheckReWriteFile , 0 , 1 , 2 , 1);
	g_signal_connect(G_OBJECT(CheckReWriteFile), "released", G_CALLBACK(SwitchWriteFile), (gpointer)uc);
	
	CheckReTime      = gtk_check_button_new_with_label("Display Receive time");
	gtk_grid_attach(GTK_GRID(Table) , CheckReTime , 0 , 2 , 2 , 1);
	g_signal_connect(G_OBJECT(CheckReTime), "released", G_CALLBACK(SwitchReceiveTime), (gpointer)uc);
	
	CheckHex         = gtk_check_button_new_with_label("Display Hex");
	gtk_grid_attach(GTK_GRID(Table) , CheckHex , 0 , 3 , 2 , 1);
	g_signal_connect(G_OBJECT(CheckHex), "released", G_CALLBACK(SwitchReceiveHex), (gpointer)uc);
	
    LabelSave = gtk_label_new ("<a href=\"null\">""<span color=\"#0266C8\">Save</span>""</a>");
    gtk_label_set_use_markup (GTK_LABEL (LabelSave), TRUE); 
    gtk_grid_attach(GTK_GRID(Table) , LabelSave , 0 , 4, 1 , 1);
	g_signal_connect(G_OBJECT(LabelSave), "activate-link", G_CALLBACK(user_function), (gpointer) uc);     

    LabelClear = gtk_label_new ("<a href=\"null\">""<span color=\"#0266C8\">Clear</span>""</a>");
    gtk_label_set_use_markup (GTK_LABEL (LabelClear), TRUE); 
    gtk_grid_attach(GTK_GRID(Table) , LabelClear , 1 , 4, 1 , 1); 
	g_signal_connect(G_OBJECT(LabelClear), "activate-link", G_CALLBACK(ClearTerminalData), (gpointer) uc);

   	Hseparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(Table) , Hseparator , 0 , 5 , 2 , 1);
    

    gtk_grid_set_row_spacing(GTK_GRID(Table), 3);
    gtk_grid_set_column_spacing(GTK_GRID(Table), 3);

}
gboolean AutoWriteUart (gpointer data)
{
    UartControl *uc = (UartControl *) data;
    int SendLen = 0;
    GtkTextIter Start,End;
    GtkTextBuffer *Buffer = NULL;
    gchar *text = NULL;
    
    Buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (uc->URC.SendText));
    gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER(Buffer), &Start, &End);
    text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(Buffer), &Start, &End, FALSE);
    SendLen = strlen(text);
    printf("text = %d\r\n",SendLen);
    if(SendLen > 0)
    {
        SendLen += 1;     
        text[strlen(text)] = '\n';
        WriteUart(text,SendLen,uc);
    }
}        
void SetAutoSend (UartControl *uc)
{
    const char *text;
    int CycleTime;
    int TimeId;
    
    text = gtk_entry_get_text(GTK_ENTRY(uc->ULC.EntrySendCycle));
    CycleTime = atoi(text);
    if(CycleTime == 0)
    {
        MessageReport(("Auto Send Data"),("Auto Send data fail,No setting time"),ERROR);
    } 
    else
    {        
        TimeId = g_timeout_add(CycleTime,(GSourceFunc)AutoWriteUart,uc); 
        uc->TimeId = TimeId;
    }
}        
void RemoveAutoSend(UartControl *uc)
{
    if(uc->TimeId > 0)
        g_source_remove(uc->TimeId);
}        
void SwitchAutoSend(GtkWidget *Check,gpointer  data)
{
	UartControl *uc = (UartControl *) data;

    if(uc->AutoSend == 0)
    {
        uc->AutoSend = 1;
        gtk_widget_set_sensitive(uc->ULC.CheckUseFile, FALSE);
        gtk_widget_set_sensitive(uc->ULC.EntrySendCycle, FALSE);
        gtk_widget_set_sensitive(uc->UDC.Button,FALSE);
        SetAutoSend(uc);
    }
    else
    {
        uc->AutoSend = 0;
        gtk_widget_set_sensitive(uc->ULC.CheckUseFile, TRUE);
        gtk_widget_set_sensitive(uc->ULC.EntrySendCycle,TRUE);
        gtk_widget_set_sensitive(uc->UDC.Button,TRUE);
        RemoveAutoSend(uc);
    }        
	
}

void SwitchAutoClean(GtkWidget *Check,gpointer  data)
{
	UartControl *uc = (UartControl *) data;
	if(uc->AutoCleanSendData == 0)
		uc->AutoCleanSendData = 1;
	else
		uc->AutoCleanSendData = 0;		
}

void SwitchUseFile(GtkWidget *Check,gpointer  data)
{
	UartControl *uc = (UartControl *) data;
    if(uc->UseFile == 0)
    {
        uc->ChooseFile = CHOOSE; 
		LoadFile(uc);
        uc->UseFile = 1;
        gtk_widget_set_sensitive(uc->ULC.CheckAutoSend, FALSE);
    }        
    else
    {
        uc->UseFile = 0;
        gtk_widget_set_sensitive(uc->ULC.CheckAutoSend, TRUE);
    }        
}

void SendFile (GtkLabel *label,
               			gchar    *uri,
               			gpointer  user_data)
{
	UartControl *uc = (UartControl *) user_data;
}
void ClearSendData (GtkLabel *label,
               			gchar    *uri,
               			gpointer  user_data)
{
	UartControl *uc = (UartControl *) user_data;
    CleanSendDate(uc);
}
void SendSet(GtkWidget *Hbox,UartControl *uc)
{
	GtkWidget *CheckAutoSend;
	GtkWidget *CheckUseFile;
	GtkWidget *CheckAutoClean;
	GtkWidget *Hseparator;
	GtkWidget *Vpaned;
	GtkWidget *SendFrame;
    GtkWidget *EntrySendCycle;
    GtkWidget *LabelSendFile;
    GtkWidget *LabelClear;
	GtkWidget *LabelSpace;
	GtkWidget *Table;
	
	
    Vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
	gtk_container_add (GTK_CONTAINER (Hbox), Vpaned);
	SendFrame = gtk_frame_new ("Send Setting");
	gtk_frame_set_label_align(GTK_FRAME(SendFrame),0.5,0.3);
    SetWidgetStyle(SendFrame,"black",12);
	gtk_paned_pack1 (GTK_PANED (Vpaned), SendFrame, FALSE, TRUE);
	
	Table = gtk_grid_new();
    gtk_container_add (GTK_CONTAINER (Vpaned), Table);
	gtk_grid_set_column_homogeneous(GTK_GRID(Table),TRUE);
	
	LabelSpace = gtk_label_new(" ");
	gtk_grid_attach(GTK_GRID(Table) ,LabelSpace , 0 , 0 , 2 , 1);

	CheckUseFile = gtk_check_button_new_with_label("Use File Data");
	gtk_grid_attach(GTK_GRID(Table) , CheckUseFile , 0 , 1 , 2 , 1);
    uc->ULC.CheckUseFile = CheckUseFile;
	g_signal_connect(G_OBJECT(CheckUseFile), 
                    "released", 
                    G_CALLBACK(SwitchUseFile), 
                    (gpointer) uc);
	
	CheckAutoClean      = gtk_check_button_new_with_label("Auto Empty");
	gtk_grid_attach(GTK_GRID(Table) , CheckAutoClean , 0 , 2 ,2 , 1);
	g_signal_connect(G_OBJECT(CheckAutoClean), 
                    "released", 
                    G_CALLBACK(SwitchAutoClean), 
                    (gpointer) uc);
	
	CheckAutoSend        = gtk_check_button_new_with_label("Auto Send Cycle");	
    uc->ULC.CheckAutoSend = CheckAutoSend;
    gtk_grid_attach(GTK_GRID(Table) , CheckAutoSend , 0 , 3, 2 , 1);
    g_signal_connect(G_OBJECT(CheckAutoSend), 
                    "released", 
                    G_CALLBACK(SwitchAutoSend), 
                    (gpointer) uc);
    
    EntrySendCycle = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(EntrySendCycle),6);
    gtk_entry_set_text(GTK_ENTRY(EntrySendCycle),"1000");
    uc->ULC.EntrySendCycle = EntrySendCycle;
    gtk_grid_attach(GTK_GRID(Table) , EntrySendCycle , 0 , 4, 2 , 1);
    
    LabelSendFile= gtk_label_new ("<a href=\"null\">""<span color=\"#0266C8\">Send File</span>""</a>");
    gtk_label_set_use_markup (GTK_LABEL (LabelSendFile), TRUE);
    g_signal_connect(G_OBJECT(LabelSendFile), "activate-link", G_CALLBACK(SendFile), (gpointer) uc); 
    gtk_grid_attach(GTK_GRID(Table) , LabelSendFile , 0 , 5, 1 , 1);
    
    LabelClear= gtk_label_new ("<a href=\"null\">""<span color=\"#0266C8\">Clear</span>""</a>");
    gtk_label_set_use_markup (GTK_LABEL (LabelClear), TRUE); 
    g_signal_connect(G_OBJECT(LabelClear), "activate-link", G_CALLBACK(ClearSendData), (gpointer) uc);
    gtk_grid_attach(GTK_GRID(Table) , LabelClear , 1 , 5, 1 , 1);

   	Hseparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(Table) , Hseparator , 0 , 6 , 2 , 1);
    
    gtk_grid_set_row_spacing(GTK_GRID(Table), 3);
    gtk_grid_set_column_spacing(GTK_GRID(Table), 3);

}
GtkWidget *CreateUartSetFace(GtkWidget *Hbox,UartControl *uc)
{
	GtkWidget *Vbox;
	
    Vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);	
	gtk_box_pack_start (GTK_BOX (Hbox), Vbox, FALSE, FALSE, 0);
	
	SerialPortSetup(Vbox,uc);	
    ReceiveSet(Vbox,uc);
    SendSet(Vbox,uc);	
	
}

void CreateReceiveFace(GtkWidget *Vpaned,UartControl *uc)
{
    GtkWidget *ReveTerminal;
	GtkWidget *Frame;
	GtkWidget *Scrolled;

	Frame = gtk_frame_new ("Receive Data From Serial");
	gtk_frame_set_label_align(GTK_FRAME(Frame),0.5,0.3);
	gtk_paned_pack1 (GTK_PANED (Vpaned),Frame, FALSE, TRUE);
	gtk_widget_set_size_request (Frame, -1, 430);

	Scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (Frame), Scrolled);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (Scrolled), 
                                    GTK_POLICY_NEVER, 
                                    GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (Scrolled), 
                                         GTK_SHADOW_IN);

	ReveTerminal = vte_terminal_new();
	uc->URC.ReveTerminal = ReveTerminal;
    vte_terminal_set_backspace_binding(VTE_TERMINAL(ReveTerminal),
				                       VTE_ERASE_ASCII_BACKSPACE);
    vte_terminal_set_input_enabled(VTE_TERMINAL(ReveTerminal),1);
    vte_terminal_set_scroll_on_output(VTE_TERMINAL(ReveTerminal),0);
    vte_terminal_set_mouse_autohide(VTE_TERMINAL(ReveTerminal),1);

    vte_terminal_set_allow_bold(VTE_TERMINAL(ReveTerminal),TRUE);
    gtk_container_add (GTK_CONTAINER (Scrolled), ReveTerminal);


}
static void insert_link (GtkTextBuffer *buffer,
                         GtkTextIter   *iter,
                         const char         *text)
{
  GtkTextTag *tag;

  tag = gtk_text_buffer_create_tag (buffer, NULL,
                                    "foreground", "blue",
                                    "underline", PANGO_UNDERLINE_SINGLE,
                                     NULL);
  gtk_text_buffer_insert_with_tags (buffer, iter, text, -1, tag, NULL);
}
void CreateSendFace(GtkWidget *Vpaned,UartControl *uc)
{
    GtkWidget *Frame;
	GtkWidget *Scrolled;
	GtkWidget *SendText;
    GtkTextIter Iter;
    GtkTextIter Start,End;
	GtkTextBuffer *Buffer;
    const char *text = "https://github.com/zhuyaliang/";

	Frame = gtk_frame_new ("Send Data Serial");
	gtk_frame_set_label_align(GTK_FRAME(Frame),0.5,0.3);
	gtk_paned_pack2 (GTK_PANED (Vpaned), Frame, FALSE, TRUE);
	gtk_widget_set_size_request (Frame, -1, 154);
    
    Scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (Frame), Scrolled);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (Scrolled), 
                                    GTK_POLICY_NEVER, 
                                    GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (Scrolled),
                                         GTK_SHADOW_IN);

	SendText = gtk_text_view_new ();
	uc->URC.SendText = SendText;

    Buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (SendText));
    gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER(Buffer), &Iter, 0);
    //获得缓冲区开始和结束位置的Iter
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(Buffer),&Start,&End);
    gtk_text_buffer_insert (GTK_TEXT_BUFFER(Buffer), &Iter, "Welcome ", -1);
    insert_link (Buffer,&Iter,text);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (SendText), GTK_WRAP_CHAR);
	gtk_container_add (GTK_CONTAINER (Scrolled), SendText);

}

GtkWidget *CreateUartTextFace(GtkWidget *Hbox,UartControl *uc)
{
	GtkWidget *Vbox;
	GtkWidget *Vpaned;
	
	Vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start (GTK_BOX (Hbox), Vbox, FALSE, FALSE, 0);
	gtk_widget_set_size_request (Vbox, 470,-1);
	Vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
	gtk_container_add (GTK_CONTAINER (Vbox), Vpaned);
  
    CreateReceiveFace(Vpaned,uc);
    
    CreateSendFace(Vpaned,uc);
        
    return Vbox;
}
void CleanSendDate(UartControl *uc)
{
    GtkTextBuffer *Buffer = NULL;
   
    Buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (uc->URC.SendText)); 
    gtk_text_buffer_set_text (GTK_TEXT_BUFFER(Buffer), (""), -1);	
}        
int WriteUart(const char *WriteData,int WriteLen,UartControl *uc)
{
    int Ret;

    g_mutex_lock (&Mutex);
    while(CurrentData == 0) //防止意外唤醒
    {    
        g_cond_wait (&Cond, &Mutex);
    }
    CurrentData = 0;
    Ret = write(Serialfd, WriteData,WriteLen);
    g_mutex_unlock (&Mutex);
    if (Ret < 0)
    {
        MessageReport(("Send Data"),("Send data fail,Send length is 0"),ERROR);
    }
    if(uc->AutoCleanSendData == 1)
    {
        CleanSendDate(uc); 
    }        
    return Ret;

}       
gpointer SendUseFileUart(gpointer data)
{
    UartControl *uc = (UartControl *) data;
    int WriteLen;
    int Ret;
    char WriteData[100] = { 0 };
    
    gtk_widget_set_sensitive(uc->UDC.Button, FALSE);
    
    while(1)
    {        
        if(uc->UseFile == 0  ||  uc->UseFilefd < 0 )
        {  
            gtk_widget_set_sensitive(uc->UDC.Button,TRUE);
            close(uc->UseFilefd);
            uc->UseFilefd = -1;
            g_thread_exit(NULL);
        }
        WriteLen = read(uc->UseFilefd,WriteData,100);
        if(WriteLen <= 0)
        {
                gtk_widget_set_sensitive(uc->UDC.Button,TRUE);
                close(uc->UseFilefd);
                uc->UseFilefd = -1;
                break;
                
        }
        else
        {
            WriteUart(WriteData,WriteLen,uc);
        }
    }
    g_thread_exit(NULL);
}        
void SendData (GtkLabel *Button,gpointer  data)
{
	UartControl *uc = (UartControl *) data;
    int SendLen = 0, ret = 0;
    GtkTextIter Start,End;
    GtkTextBuffer *Buffer = NULL;
    gchar *text = NULL;

    if (uc->UP.fd < 0) 
    {
        MessageReport(("Send Data"),("Send data fail, Serial don`t open"),ERROR);
        return;
    }
    if(uc->UseFile == 1 && uc->UseFilefd > 0 )
    {
        g_thread_new("SendUart",(GThreadFunc)SendUseFileUart,(gpointer)uc);
        
    }
    else
    {        
        Buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (uc->URC.SendText));
        gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER(Buffer), &Start, &End);
        text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(Buffer), &Start, &End, FALSE);
        SendLen = strlen(text);
        if(SendLen > 0)
        {
            SendLen += 1;     
            text[strlen(text)] = '\n';
            WriteUart(text,SendLen,uc);
        }
        else
        {
            MessageReport(("Send Data"),("Send data can not be empty"),ERROR);
        }        
    }       
}
void CreateUartBottom(GtkWidget *Vbox,UartControl *uc)
{
    GtkWidget *image;
    GtkWidget *LabelState;
    GtkWidget *LabelRx;
    GtkWidget *LabelTx;
    GtkWidget *LabelSpace;

    GtkWidget *Button;
    GtkWidget *Hbox;

    Hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start (GTK_BOX (Vbox), Hbox, FALSE, FALSE, 0);

    image = gtk_image_new_from_icon_name ("audio-speakers",
                                           GTK_ICON_SIZE_MENU);
    gtk_box_pack_start (GTK_BOX (Hbox), image, FALSE, FALSE, 0);
    LabelState = gtk_label_new("Serial Assistant");
    uc->UDC.LabelState = LabelState;
    gtk_box_pack_start (GTK_BOX (Hbox), LabelState, TRUE, FALSE, 0);

    LabelSpace = gtk_label_new("           ");
    gtk_box_pack_start (GTK_BOX (Hbox), LabelSpace, TRUE, TRUE, 0);

    LabelRx = gtk_toggle_button_new_with_label("RX:    0");
    gtk_box_pack_start (GTK_BOX (Hbox), LabelRx, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(LabelRx),TRUE);
    SetWidgetStyle(LabelRx,"blue",11);
    uc->UDC.LabelRx = LabelRx;

    LabelTx = gtk_toggle_button_new_with_label("TX:    0");
    gtk_box_pack_start (GTK_BOX (Hbox), LabelTx, TRUE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(LabelTx),TRUE);
    SetWidgetStyle(LabelTx,"blue",11);
    uc->UDC.LabelTx = LabelTx;
    
    Button = gtk_button_new_with_label ("Send");
    SetWidgetStyle(Button,"black",11);
    gtk_box_pack_start (GTK_BOX (Hbox), Button, TRUE, FALSE, 0);
    uc->UDC.Button = Button;
    g_signal_connect(G_OBJECT(Button), "clicked", G_CALLBACK(SendData), uc);

}        

void CreateFace (GtkWidget *Vbox,UartControl *uc)
{
	GtkWidget *Hbox;
	
	Hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);	
    gtk_box_pack_start (GTK_BOX (Vbox), Hbox, FALSE, FALSE, 0);
    
   	CreateUartSetFace(Hbox,uc);
	CreateUartTextFace(Hbox,uc);
    CreateUartBottom(Vbox,uc);
	
}

void SetDefaultSerial(UartControl *uc)
{
	const char *text = "/dev/ttyS0";
	memset(uc->UP.UartPort,'\0',30);
	memcpy(uc->UP.UartPort,text,strlen(text));
	uc->UP.UartBaud = 9600;
	uc->UP.UartParity = 0;
	uc->UP.UartStop = 1;
	uc->UP.UartData = 8;
	uc->UP.fd      = -1;
	uc->ShowHex     = 0;
	uc->Filefd     = -1;
    uc->UseFilefd  = -1;
	uc->Redirect    = 0;
    uc->ShowTime    = 0;
    uc->AutoCleanSendData = 0; 
    uc->UseFile     = 0;	
    uc->AutoSend    = 0;
    uc->TimeId      = -1;
}
int main(int argc, char **argv)
{
	GtkWidget *Vbox;
	UartControl uc;
	 
  	gdk_threads_init(); 
    g_mutex_init(&Mutex);
    g_cond_init (&Cond);
    gtk_init(&argc, &argv);
   
    MainWindow = CreateMainWindow();
	uc.MainWindow = MainWindow;
	
    Vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);	
	gtk_container_add (GTK_CONTAINER (MainWindow), Vbox);
    SetDefaultSerial(&uc);
	CreateFace(Vbox,&uc);
    gtk_widget_show_all(MainWindow);
    gtk_main ();  
}
