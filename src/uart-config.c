#include "config.h"
#include "uart-config.h"
#include "uart-share.h"
#include "uart-init.h"
#include "uart-read.h"

static int SelectChange;
static gboolean
SetSerialNotifyEvent (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data)
{
	UartControl *uc = (UartControl *) data;
    gtk_label_set_text(GTK_LABEL(uc->UDC.LabelState),\
                      _("Serial Setting and open Serial"));

    return TRUE;
} 
static void SwitchUartPort(GtkWidget *widget,gpointer data)
{
    UartControl *uc = (UartControl *) data;
	char *text;
    GtkTreeIter   iter;
    GtkTreeModel *model;
    char Path[30] = { 0 };
    if(SelectChange == 0)
    {    
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
static int gnSwitchSatue = 1;
static int GetSwitchState( const char *ButtonName)
{
    if(gnSwitchSatue == 0)
    { 
        gnSwitchSatue = 1;       
        return 0;
    }    
    else
    {   
        gnSwitchSatue = 0;
        return 1;
    }
}
static void LockSetSerial(UartControl *uc)
{
	gtk_widget_set_sensitive(uc->ULC.SelectPort, FALSE);
	gtk_widget_set_sensitive(uc->ULC.SelectBaud, FALSE);
	gtk_widget_set_sensitive(uc->ULC.SelectParity, FALSE);
	gtk_widget_set_sensitive(uc->ULC.SelectStop, FALSE);
	gtk_widget_set_sensitive(uc->ULC.SelectData, FALSE);
	
}     	

static void UnLockSetSerial(UartControl *uc)
{
	gtk_widget_set_sensitive(uc->ULC.SelectPort, TRUE);
	gtk_widget_set_sensitive(uc->ULC.SelectBaud, TRUE);
	gtk_widget_set_sensitive(uc->ULC.SelectParity, TRUE);
	gtk_widget_set_sensitive(uc->ULC.SelectStop, TRUE);
	gtk_widget_set_sensitive(uc->ULC.SelectData, TRUE);
}
static void ResetSerial(UartControl *uc)
{
    ThreadEnd = 1;   //关闭读串口数据线程

    close(Serialfd);  //关闭串口描述符
    Serialfd = -1;    
    uc->UP.fd = -1;

    if(uc->Redirect == 1 && uc->Filefd > 0)  //关闭接收内容重定向
    {
        close(uc->Filefd);
        uc->Filefd = -1;
        uc->Redirect = 0;
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(uc->ULC.CheckReWriteFile),
                                      FALSE);
    }
    if(uc->ShowTime == 1)
    {
        uc->ShowTime = 0;
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(uc->ULC.CheckReTime),
                                      FALSE);
    } 
    if(uc->ShowHex == 1)
    {
       uc->ShowHex = 0;
       gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(uc->ULC.CheckHex),
                                      FALSE);

    }        
    if(uc->UseFile == 1  &&  uc->UseFilefd > 0 )
    {
        if(uc->UseFilefd > 0)
            close(uc->UseFilefd);
        uc->UseFilefd = -1;
        uc->UseFile = 0;
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(uc->ULC.CheckUseFile),
                                      FALSE);

    }
    if(uc->AutoCleanSendData == 1)
    {
        uc->AutoCleanSendData = 0;
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(uc->ULC.CheckAutoClean),
                                      FALSE);
    }        
    if(uc->AutoSend ==1  && uc->TimeId > 0)
    {        
        g_source_remove(uc->TimeId);  
        uc->TimeId = -1;
        uc->AutoSend = 0;
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(uc->ULC.CheckAutoSend),
                                      FALSE);
        gtk_widget_set_sensitive(uc->ULC.CheckUseFile, TRUE);
        gtk_widget_set_sensitive(uc->ULC.EntrySendCycle,TRUE);
        gtk_widget_set_sensitive(uc->UDC.Button,TRUE);
    }
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
        	MessageReport(_("close Serial"),_("close Serial Fail"),ERROR);
        }
        else
        {
        	UnLockSetSerial(uc);
            ResetSerial(uc);
        	gtk_button_set_label(GTK_BUTTON(Button),_("● Open"));
        	SetWidgetStyle(Button,"black",13);
        }
    }
    else
    {
        OpenState = InitSerial(uc);
        if(OpenState < 0)
        {   
            gnSwitchSatue = 1;      
        	MessageReport(_("Open Serial"),_("Open Serial Fail"),ERROR);
        }
        else
        {	ThreadEnd = 0;
   			LockSetSerial(uc);
   			uc->UP.fd =  OpenState;
   			Serialfd = OpenState;
        	gtk_button_set_label(GTK_BUTTON(Button),_("● Close"));
        	SetWidgetStyle(Button,"red",13);
        	CreateReadUart(uc);

		}
    }
}

static GtkWidget *SetComboBox(GtkListStore  *Store)
{
	
	GtkCellRenderer *Renderer;
    GtkWidget *ComboUser;
    
    ComboUser = gtk_combo_box_new_with_model(GTK_TREE_MODEL(Store));
    g_object_unref(G_OBJECT(Store));
    Renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(ComboUser),Renderer,TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(ComboUser),
                                   Renderer,
                                   "text",
                                    0,NULL);
    gtk_combo_box_set_active(GTK_COMBO_BOX(ComboUser),1);

    return ComboUser;
	
}

static GSList *GetDevice (void)
{
    DIR *dp;
    struct stat statbuf;
    struct dirent *entry;
    GSList *list = NULL;
 
    if((dp = opendir(DEVICEFILE)) == NULL)
         return NULL;
    while((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name,&statbuf);
        if(S_ISDIR(statbuf.st_mode))
        {
            if(strcmp(".",entry->d_name) == 0 ||strcmp("..",entry->d_name) == 0)
            continue;
        }
        if(g_strrstr(entry->d_name,"ttyS") != NULL ||
           g_strrstr(entry->d_name,"ttyUSB") != NULL)
        {
            list = g_slist_append(list,entry->d_name);
        }    
    }
    return list;
}   
static gboolean CheckDeviceIsUpdate (GSList *NewList,
                                     GSList *OldList)
{
    GSList *new, *old;
    int i = 0;
    if(NewList == NULL || OldList == NULL)
        return TRUE;
    if(g_slist_length(NewList) != g_slist_length(OldList))
        return TRUE;
    
    for (new = NewList,old = OldList;
		 new; 
		 new = new->next,old = old->next,i++)
	{
		if(g_strcmp0(new->data,old->data) != 0)
			return TRUE;	
	}		
	return FALSE;
}    
static gboolean UpdateDevice (gpointer data)
{
    UartControl *uc = (UartControl *)data;
    GtkTreeIter     Iter;
    GSList *CurrentList, *l;
    int i = 0;

    CurrentList = GetDevice();
    if(CheckDeviceIsUpdate(CurrentList,uc->Portlist) == TRUE)
    {
        SelectChange = 1; 
        gtk_list_store_clear(uc->PortStore);
        uc->Portlist = g_slist_copy(CurrentList);
        for (l = uc->Portlist; l; l = l->next,i++)
        {
            gtk_list_store_append(uc->PortStore,&Iter);
            gtk_list_store_set(uc->PortStore,&Iter,0,l->data,-1);
            l->data = g_strdup (l->data);
        }
        SetComboBox(uc->PortStore);
		gtk_combo_box_set_active(GTK_COMBO_BOX(uc->ULC.SelectPort),0);
        SelectChange = 0;

    }
	g_slist_free(CurrentList);
    return TRUE;
}    
static GtkWidget *SetComPort(UartControl *uc)
{
    GtkTreeIter     Iter;
    GSList *l;
    int i = 0;
    
    uc->Portlist =  GetDevice();
    if(uc->Portlist == NULL)
    {
        MessageReport(_("Get Deivce"),
                      _("/dev/ No available devices were found"),
                      ERROR);
        exit(-1);
    }    

    uc->PortStore = gtk_list_store_new(1,G_TYPE_STRING);

    for (l = uc->Portlist; l; l = l->next,i++)
    {
        gtk_list_store_append(uc->PortStore,&Iter);
        gtk_list_store_set(uc->PortStore,&Iter,0,l->data,-1);
    }
    g_timeout_add(500,(GSourceFunc)UpdateDevice,uc);
    return SetComboBox(uc->PortStore);
}
static GtkWidget *SetBaud(void)
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

static GtkWidget *SetParity(void)
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
static GtkWidget *SetStop(void)
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
static GtkWidget *SetData(void)
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
    GtkWidget *LabelState;
	GtkWidget *Button;

	Vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
	gtk_container_add (GTK_CONTAINER (Hbox), Vpaned);

    Frame = gtk_frame_new (_("Serial Setting"));
    SetWidgetStyle(Frame,"black",12);
	gtk_frame_set_label_align(GTK_FRAME(Frame),0.5,0.3);
	gtk_paned_pack1 (GTK_PANED (Vpaned), Frame, FALSE, TRUE);

    Table = gtk_grid_new();
    gtk_container_add (GTK_CONTAINER (Vpaned), Table);
	gtk_grid_set_column_homogeneous(GTK_GRID(Table),TRUE);

    gtk_widget_add_events(Vpaned,GDK_ENTER_NOTIFY_MASK);
	g_signal_connect(G_OBJECT(Vpaned),
                    "enter-notify-event",
                     G_CALLBACK(SetSerialNotifyEvent),
                     (gpointer) uc);

    LableComPort = gtk_label_new(NULL);
    SetLableFontType(LableComPort,"black",10,_("Serial Number"));
    gtk_grid_attach(GTK_GRID(Table) , LableComPort , 0 , 0 , 1 , 1);

    SelectPort = SetComPort(uc);
    gtk_combo_box_set_active(GTK_COMBO_BOX(SelectPort),0);
    uc->ULC.SelectPort = SelectPort;
    gtk_grid_attach(GTK_GRID(Table) ,SelectPort , 1 , 0 , 1 , 1);

    g_signal_connect(G_OBJECT(SelectPort),
                    "changed",
                    G_CALLBACK(SwitchUartPort),
                    (gpointer) uc);

	LableBaud = gtk_label_new(NULL);
    SetLableFontType(LableBaud,"black",10,_("Baud Rate"));
    gtk_grid_attach(GTK_GRID(Table) , LableBaud , 0 , 1 , 1 , 1);

   	SelectBaud = SetBaud();
    gtk_combo_box_set_active(GTK_COMBO_BOX(SelectBaud),5);
    uc->ULC.SelectBaud = SelectBaud;
    gtk_grid_attach(GTK_GRID(Table) ,SelectBaud , 1 , 1 , 1 , 1);
    g_signal_connect(G_OBJECT(SelectBaud),
                    "changed",
                    G_CALLBACK(SwitchUartBaud),
                    (gpointer) uc);

	LabelParity = gtk_label_new(NULL);
    SetLableFontType(LabelParity,"black",10,_("Parity Bit"));
    gtk_grid_attach(GTK_GRID(Table) , LabelParity , 0 , 2 , 1 , 1);

   	SelectParity = SetParity();
    gtk_combo_box_set_active(GTK_COMBO_BOX(SelectParity),0);
    uc->ULC.SelectParity = SelectParity;
    gtk_grid_attach(GTK_GRID(Table) ,SelectParity , 1 , 2 , 1 , 1);
	g_signal_connect(G_OBJECT(SelectParity),
                    "changed",
                    G_CALLBACK(SwitchUartParity),
                    (gpointer) uc);

	LabelData = gtk_label_new(NULL);
    SetLableFontType(LabelData,"black",10,_("Data Bit"));
    gtk_grid_attach(GTK_GRID(Table) , LabelData , 0 , 3 , 1 , 1);

   	SelectData = SetData();
    gtk_combo_box_set_active(GTK_COMBO_BOX(SelectData),0);
    gtk_grid_attach(GTK_GRID(Table) ,SelectData , 1 , 3 , 1 , 1);
    uc->ULC.SelectData = SelectData;
	g_signal_connect(G_OBJECT(SelectData),
                    "changed",
                    G_CALLBACK(SwitchUartData),
                    (gpointer) uc);

	LabelStop = gtk_label_new(NULL);
    SetLableFontType(LabelStop,"black",10,_("Stop Bit"));
    gtk_grid_attach(GTK_GRID(Table) , LabelStop , 0 , 4 , 1 , 1);

   	SelectStop = SetStop();
    gtk_combo_box_set_active(GTK_COMBO_BOX(SelectStop),0);
    uc->ULC.SelectStop = SelectStop;
    gtk_grid_attach(GTK_GRID(Table) ,SelectStop , 1 , 4 , 1 , 1);
	g_signal_connect(G_OBJECT(SelectStop),
                    "changed",
                    G_CALLBACK(SwitchUartStop),
                    (gpointer) uc);

    LabelState = gtk_label_new(NULL);
    SetLableFontType(LabelState,"black",20,"● ");
    gtk_grid_attach(GTK_GRID(Table) , LabelState , 0 , 5 , 1 , 1);

    Button = gtk_button_new_with_label (_("Open"));
    SetWidgetStyle(Button,"black",13);
    gtk_widget_set_vexpand(Button,TRUE);
    gtk_widget_set_valign (Button,GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(Table) , Button , 1 , 5 , 1 , 1);
    g_signal_connect(G_OBJECT(Button), 
                    "clicked", 
                    G_CALLBACK(OpenSerial), 
                    (gpointer) uc);

	Hseparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(Table) , Hseparator , 0 , 7 , 2 , 1);

    gtk_grid_set_row_spacing(GTK_GRID(Table), 1);
    gtk_grid_set_column_spacing(GTK_GRID(Table), 1);
}
