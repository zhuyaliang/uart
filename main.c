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
#include <vte-2.91/vte/vte.h>
#include <sys/stat.h>
#include<gdk/gdkkeysyms.h>

#define     OPEN     1
#define     CLOSE    0


typedef struct 
{
	GtkWidget *LableComPort;	
	GtkWidget *SelectComPort;
	GtkWidget *LableBaud;
	GtkWidget *SelectBaud;
	GtkWidget *Entry;
	
}UartSetFace;

typedef struct 
{
	GtkWidget Lable;	
	
}UartTextFace;

typedef struct
{
	GtkWidget    *MainWindow;
	UartSetFace  *ULC;
	UartTextFace *URC;
	
}UartControl;

int gnSwitchSatue;
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
	gtk_widget_set_size_request(WindowMain, 600, 500);
    pixbuf = gdk_pixbuf_new_from_file ("xgcom.png", &error);
    gtk_window_set_icon (GTK_WINDOW (WindowMain), pixbuf);
	
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
    gtk_list_store_set(Store,&Iter,0,"ttyS1",-1);

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
static void OpenSerial(GtkWidget *Button,gpointer user_data)
{
    int SwitchState;

    SwitchState = GetSwitchState(gtk_button_get_label(GTK_BUTTON(Button)));
    if(SwitchState == CLOSE)
    {
        gtk_button_set_label(GTK_BUTTON(Button),"● Open");
        SetWidgetStyle(Button,"black",13);
    
    }
    else
    {        
        gtk_button_set_label(GTK_BUTTON(Button),"● Close");
        SetWidgetStyle(Button,"red",13);
    }    

    printf("aaaaaaaaaaaaaa\r\n");
}        
void SerialPortSetup(GtkWidget *Hbox)
{
	GtkWidget *Vpaned;
	GtkWidget *Frame;
	GtkWidget *Table;
	GtkWidget *LableComPort;
	GtkWidget *SelectComPort;
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
   	
    SelectComPort = SetComPort();
    gtk_combo_box_set_active(GTK_COMBO_BOX(SelectComPort),1);
    gtk_grid_attach(GTK_GRID(Table) ,SelectComPort , 1 , 0 , 1 , 1);

	LableBaud = gtk_label_new(NULL);
    SetLableFontType(LableBaud,"black",10,("Baud Rate"));
    gtk_grid_attach(GTK_GRID(Table) , LableBaud , 0 , 1 , 1 , 1);
	
   	SelectBaud = SetBaud();
    gtk_combo_box_set_active(GTK_COMBO_BOX(SelectBaud),1);
    gtk_grid_attach(GTK_GRID(Table) ,SelectBaud , 1 , 1 , 1 , 1);
    
	LabelParity = gtk_label_new(NULL);
    SetLableFontType(LabelParity,"black",10,("Parity Bit"));
    gtk_grid_attach(GTK_GRID(Table) , LabelParity , 0 , 2 , 1 , 1);
	
   	SelectParity = SetParity();
    gtk_combo_box_set_active(GTK_COMBO_BOX(SelectParity),1);
    gtk_grid_attach(GTK_GRID(Table) ,SelectParity , 1 , 2 , 1 , 1);

	LabelData = gtk_label_new(NULL);
    SetLableFontType(LabelData,"black",10,("Data Bit"));
    gtk_grid_attach(GTK_GRID(Table) , LabelData , 0 , 3 , 1 , 1);
	
   	SelectData = SetData();
    gtk_combo_box_set_active(GTK_COMBO_BOX(SelectData),1);
    gtk_grid_attach(GTK_GRID(Table) ,SelectData , 1 , 3 , 1 , 1);

	LabelStop = gtk_label_new(NULL);
    SetLableFontType(LabelStop,"black",10,("Stop Bit"));
    gtk_grid_attach(GTK_GRID(Table) , LabelStop , 0 , 4 , 1 , 1);
	
   	SelectStop = SetStop();
    gtk_combo_box_set_active(GTK_COMBO_BOX(SelectStop),1);
    gtk_grid_attach(GTK_GRID(Table) ,SelectStop , 1 , 4 , 1 , 1);
   
    Button = gtk_button_new_with_label ("● open");
    SetWidgetStyle(Button,"black",13);
    gtk_grid_attach(GTK_GRID(Table) , Button , 0 , 5 , 2 , 1);
    g_signal_connect(G_OBJECT(Button), "clicked", G_CALLBACK(OpenSerial), NULL);  
     
	Hseparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(Table) , Hseparator , 0 , 7 , 2 , 1);
    

    gtk_grid_set_row_spacing(GTK_GRID(Table), 1);
    gtk_grid_set_column_spacing(GTK_GRID(Table), 1);
}
void ReceiveSet(GtkWidget *Hbox)
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
	CheckReTime      = gtk_check_button_new_with_label("Display Receive time");
	gtk_grid_attach(GTK_GRID(Table) , CheckReTime , 0 , 2 , 2 , 1);
	CheckHex         = gtk_check_button_new_with_label("Display Hex");
	gtk_grid_attach(GTK_GRID(Table) , CheckHex , 0 , 3 , 2 , 1);
	CheckStop        = gtk_check_button_new_with_label("DisPlay stop");
    gtk_grid_attach(GTK_GRID(Table) , CheckStop , 0 , 4, 2 , 1);
    
    LabelSave = gtk_label_new ("<a href=\"null\">""<span color=\"#0266C8\">Save</span>""</a>");
    gtk_label_set_use_markup (GTK_LABEL (LabelSave), TRUE); 
    gtk_grid_attach(GTK_GRID(Table) , LabelSave , 0 , 5, 1 , 1);

    LabelClear = gtk_label_new ("<a href=\"null\">""<span color=\"#0266C8\">Clear</span>""</a>");
    gtk_label_set_use_markup (GTK_LABEL (LabelClear), TRUE); 
    gtk_grid_attach(GTK_GRID(Table) , LabelClear , 1 , 5, 1 , 1); 

   	Hseparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(Table) , Hseparator , 0 , 6 , 2 , 1);
    

    gtk_grid_set_row_spacing(GTK_GRID(Table), 3);
    gtk_grid_set_column_spacing(GTK_GRID(Table), 3);

}
void SendSet(GtkWidget *Hbox)
{
	GtkWidget *CheckReWriteFile;
	GtkWidget *CheckReTime;
	GtkWidget *CheckHex;
	GtkWidget *CheckStop;
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

	CheckReWriteFile = gtk_check_button_new_with_label("Use File Data");
	gtk_grid_attach(GTK_GRID(Table) , CheckReWriteFile , 0 , 1 , 2 , 1);
	
	CheckReTime      = gtk_check_button_new_with_label("Auto Empty");
	gtk_grid_attach(GTK_GRID(Table) , CheckReTime , 0 , 2 ,2 , 1);
	
	CheckHex         = gtk_check_button_new_with_label("Send Hex");
	gtk_grid_attach(GTK_GRID(Table) , CheckHex , 0 , 3 , 2 , 1);
	
	CheckStop        = gtk_check_button_new_with_label("Auto Send Cycle");	
    gtk_grid_attach(GTK_GRID(Table) , CheckStop , 0 , 4, 2 , 1);
    
    EntrySendCycle = gtk_entry_new();
    gtk_widget_set_size_request (EntrySendCycle, 40, -1);
    gtk_entry_set_invisible_char (GTK_ENTRY (EntrySendCycle), 9679);
    gtk_entry_set_max_length(GTK_ENTRY(EntrySendCycle),6);
    gtk_entry_set_text(GTK_ENTRY(EntrySendCycle),"1000");
    gtk_grid_attach(GTK_GRID(Table) , EntrySendCycle , 0 , 5, 2 , 1);
    
    LabelSendFile= gtk_label_new ("<a href=\"null\">""<span color=\"#0266C8\">Send File</span>""</a>");
    gtk_label_set_use_markup (GTK_LABEL (LabelSendFile), TRUE); 
    gtk_grid_attach(GTK_GRID(Table) , LabelSendFile , 0 , 6, 1 , 1);
    
    LabelClear= gtk_label_new ("<a href=\"null\">""<span color=\"#0266C8\">Clear</span>""</a>");
    gtk_label_set_use_markup (GTK_LABEL (LabelClear), TRUE); 
    gtk_grid_attach(GTK_GRID(Table) , LabelClear , 1 , 6, 1 , 1);

   	Hseparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(Table) , Hseparator , 0 , 7 , 2 , 1);
    
    gtk_grid_set_row_spacing(GTK_GRID(Table), 3);
    gtk_grid_set_column_spacing(GTK_GRID(Table), 3);

}
GtkWidget *CreateUartSetFace(GtkWidget *Hbox,UartSetFace *ULC)
{
	GtkWidget *Vbox;
	
    Vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);	
	gtk_box_pack_start (GTK_BOX (Hbox), Vbox, FALSE, FALSE, 0);
	SerialPortSetup(Vbox);	
    ReceiveSet(Vbox);
    SendSet(Vbox);	
	
}

void CreateReceiveFace(GtkWidget *Vpaned)
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
	
    vte_terminal_set_backspace_binding(VTE_TERMINAL(ReveTerminal),
				                       VTE_ERASE_ASCII_BACKSPACE);
    vte_terminal_set_input_enabled(VTE_TERMINAL(ReveTerminal),1);
    vte_terminal_set_scroll_on_output(VTE_TERMINAL(ReveTerminal),TRUE);
    vte_terminal_set_mouse_autohide(VTE_TERMINAL(ReveTerminal),1);
/*
    int fd;
    char buf[10240] = { 0 };
    fd = open("./main.c",O_RDONLY);
    read(fd,buf,10240);
	vte_terminal_feed(VTE_TERMINAL(ReveTerminal), buf, strlen(buf));
*/    gtk_container_add (GTK_CONTAINER (Scrolled), ReveTerminal);


}

void CreateSendFace(GtkWidget *Vpaned)
{
    GtkWidget *Frame;
	GtkWidget *Scrolled;
	GtkWidget *SendText;

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
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (SendText), GTK_WRAP_CHAR);
	gtk_container_add (GTK_CONTAINER (Scrolled), SendText);


}

GtkWidget *CreateUartTextFace(GtkWidget *Hbox,UartTextFace *URC)
{
	GtkWidget *Vbox;
	GtkWidget *Vpaned;
	
	Vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start (GTK_BOX (Hbox), Vbox, FALSE, FALSE, 0);
	gtk_widget_set_size_request (Vbox, 470,-1);
	Vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
	gtk_container_add (GTK_CONTAINER (Vbox), Vpaned);
  
    CreateReceiveFace(Vpaned);
    
    CreateSendFace(Vpaned);
        
    return Vbox;
}
void CreateUartBottom(GtkWidget *Vbox)
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
    gtk_box_pack_start (GTK_BOX (Hbox), LabelState, TRUE, FALSE, 0);

    LabelSpace = gtk_label_new("           ");
    gtk_box_pack_start (GTK_BOX (Hbox), LabelSpace, TRUE, TRUE, 0);

    LabelRx = gtk_toggle_button_new_with_label("RX:    ");
    gtk_box_pack_start (GTK_BOX (Hbox), LabelRx, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(LabelRx),TRUE);
    SetWidgetStyle(LabelRx,"blue",11);

    LabelTx = gtk_toggle_button_new_with_label("TX:    ");
    gtk_box_pack_start (GTK_BOX (Hbox), LabelTx, TRUE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(LabelTx),TRUE);
    SetWidgetStyle(LabelTx,"blue",11);
    
    Button = gtk_button_new_with_label ("Send");
    SetWidgetStyle(Button,"black",11);
    gtk_box_pack_start (GTK_BOX (Hbox), Button, TRUE, FALSE, 0);

}        

void CreateFace (GtkWidget *Vbox)
{
	GtkWidget *Hbox;
	GtkWidget *Hbox1;
	GtkWidget *LeftSetBox;
	GtkWidget *RightTextBox;
	UartControl uc;
	
	Hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);	
    gtk_box_pack_start (GTK_BOX (Vbox), Hbox, FALSE, FALSE, 0);
   	LeftSetBox = CreateUartSetFace(Hbox,uc.ULC);
	RightTextBox = CreateUartTextFace(Hbox,uc.URC);
    CreateUartBottom(Vbox);
	
}
int main(int argc, char **argv)
{
	GtkWidget *MainWindow;
	GtkWidget *Vbox;
	UartControl uc;
    gtk_init(&argc, &argv);
   
    MainWindow = CreateMainWindow();

    GdkPixbuf *Xgcom_icon_pixbuf;
    GError *error = NULL;
    Xgcom_icon_pixbuf = gdk_pixbuf_new_from_file ("xgcom.png", &error);
    gtk_window_set_icon (GTK_WINDOW (MainWindow), Xgcom_icon_pixbuf);

    Vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);	
	gtk_container_add (GTK_CONTAINER (MainWindow), Vbox);
    uc.MainWindow = MainWindow;
	CreateFace(Vbox);
    gtk_widget_show_all(MainWindow);
    gtk_main ();  
    

}
