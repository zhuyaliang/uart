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

typedef struct 
{
	GtkWidget *LableComPort;	
	GtkWidget *SelectComPort;
	GtkWidget *LableBaud;
	GtkWidget *SelectBaud;
	
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
	GtkWidget *LabelSpace;
	
	Vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
	gtk_container_add (GTK_CONTAINER (Hbox), Vpaned);
	Frame = gtk_frame_new ("Serial Setting");
	gtk_frame_set_label_align(GTK_FRAME(Frame),0.5,0.3);
	gtk_widget_show (Frame);
	gtk_paned_pack1 (GTK_PANED (Vpaned), Frame, FALSE, TRUE);
	
    Table = gtk_grid_new();
    gtk_container_add (GTK_CONTAINER (Frame), Table);
    gtk_box_pack_start (GTK_BOX (Hbox), Table, FALSE, FALSE, 0);
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
   
    Button = gtk_toggle_button_new_with_label("open");
    gtk_grid_attach(GTK_GRID(Table) , Button , 0 , 5 , 2 , 1);
    
    LabelSpace = gtk_label_new(" ");
    gtk_grid_attach(GTK_GRID(Table) , LabelSpace , 0 , 6 , 2 , 1);
    
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
	GtkWidget *rcv_frame;
	GtkWidget *LabelSpace;
	GtkWidget *Table;
	
	Vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
	gtk_container_add (GTK_CONTAINER (Hbox), Vpaned);
	rcv_frame = gtk_frame_new ("Receive Setting");
	gtk_frame_set_label_align(GTK_FRAME(rcv_frame),0.5,0.3);
	gtk_widget_show (rcv_frame);
	gtk_paned_pack1 (GTK_PANED (Vpaned), rcv_frame, FALSE, TRUE);
	
	Table = gtk_grid_new();
	gtk_container_add (GTK_CONTAINER (rcv_frame), Table);
    gtk_box_pack_start (GTK_BOX (Hbox), Table, FALSE, FALSE, 0);
	gtk_grid_set_column_homogeneous(GTK_GRID(Table),TRUE);

	LabelSpace = gtk_label_new(" ");
	gtk_grid_attach(GTK_GRID(Table) ,LabelSpace , 0 , 0 , 1 , 1);
	
	CheckReWriteFile = gtk_check_button_new_with_label("Receive Write File");
	gtk_grid_attach(GTK_GRID(Table) , CheckReWriteFile , 0 , 1 , 1 , 1);
	CheckReTime      = gtk_check_button_new_with_label("Display Receive time");
	gtk_grid_attach(GTK_GRID(Table) , CheckReTime , 0 , 2 , 1 , 1);
	CheckHex         = gtk_check_button_new_with_label("Display Hex");
	gtk_grid_attach(GTK_GRID(Table) , CheckHex , 0 , 3 , 1 , 1);
	CheckStop        = gtk_check_button_new_with_label("DisPlay stop");
    gtk_grid_attach(GTK_GRID(Table) , CheckStop , 0 , 4, 1 , 1);
    
    LabelSpace = gtk_label_new(" ");
    gtk_grid_attach(GTK_GRID(Table) , LabelSpace , 0 , 5 , 1 , 1);
    
   	Hseparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(Table) , Hseparator , 0 , 6 , 1 , 1);
    

    gtk_grid_set_row_spacing(GTK_GRID(Table), 3);
    gtk_grid_set_column_spacing(GTK_GRID(Table), 3);

}
static void SetFontSize(GtkWidget *win,int Size)
{
    PangoFontDescription *font_desc = pango_font_description_from_string ("Serif 13");
    pango_font_description_set_size (font_desc, Size * PANGO_SCALE);

    gtk_widget_override_font(win,font_desc);
   // gtk_widget_modify_font (win, font_desc);
    pango_font_description_free (font_desc);
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
	GtkWidget *LabelSpace;
	GtkWidget *LabelSpace1;
	GtkWidget *Table;
	
	Vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
	gtk_container_add (GTK_CONTAINER (Hbox), Vpaned);
	SendFrame = gtk_frame_new ("Send Setting");
	gtk_frame_set_label_align(GTK_FRAME(SendFrame),0.5,0.3);
	//SetFontSize(SendFrame,12);
	gtk_widget_show (SendFrame);
	gtk_paned_pack1 (GTK_PANED (Vpaned), SendFrame, FALSE, TRUE);
	
	Table = gtk_grid_new();
	gtk_container_add (GTK_CONTAINER (SendFrame), Table);
    gtk_box_pack_start (GTK_BOX (Hbox), Table, FALSE, FALSE, 0);
	gtk_grid_set_column_homogeneous(GTK_GRID(Table),TRUE);
	
	LabelSpace = gtk_label_new(" ");
	gtk_grid_attach(GTK_GRID(Table) ,LabelSpace , 0 , 0 , 1 , 1);

	CheckReWriteFile = gtk_check_button_new_with_label("Receive Write File");
	gtk_grid_attach(GTK_GRID(Table) , CheckReWriteFile , 0 , 1 , 1 , 1);
	
	CheckReTime      = gtk_check_button_new_with_label("Display Receive time");
	gtk_grid_attach(GTK_GRID(Table) , CheckReTime , 0 , 2 , 1 , 1);
	
	CheckHex         = gtk_check_button_new_with_label("Display Hex");
	gtk_grid_attach(GTK_GRID(Table) , CheckHex , 0 , 3 , 1 , 1);
	
	CheckStop        = gtk_check_button_new_with_label("DisPlay stop");	
    gtk_grid_attach(GTK_GRID(Table) , CheckStop , 0 , 4, 1 , 1);
    
    LabelSpace1 = gtk_label_new(" ");
    gtk_grid_attach(GTK_GRID(Table) , LabelSpace1 , 0 , 5 , 1 , 1);
 
   	Hseparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(Table) , Hseparator , 0 , 6 , 1 , 1);
    
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

GtkWidget *CreateUartTextFace(GtkWidget *Hbox,UartTextFace *URC)
{
	GtkWidget *right_vbox;
	GtkWidget *rcv_text;
	GtkWidget *right_vpaned;
	GtkWidget *rcv_frame;
	GtkWidget *alignment2;
	GtkWidget *scrolledwindow2;
	GtkWidget *rcv_label;
	GtkWidget *send_frame;
	GtkWidget *alignment1;
	GtkWidget *scrolledwindow1;
	GtkWidget *send_text;
	GtkWidget *send_label;
	
	right_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (right_vbox);
	gtk_box_pack_start (GTK_BOX (Hbox), right_vbox, FALSE, FALSE, 0);
	gtk_widget_set_size_request (right_vbox, 500, -1);

	right_vpaned = gtk_vpaned_new ();
	gtk_widget_show (right_vpaned);
	gtk_container_add (GTK_CONTAINER (right_vbox), right_vpaned);
  
	rcv_frame = gtk_frame_new (NULL);
	gtk_widget_show (rcv_frame);
	//gtk_box_pack_start (GTK_BOX (right_vbox), rcv_frame, TRUE, TRUE, 0);
	gtk_paned_pack1 (GTK_PANED (right_vpaned), rcv_frame, FALSE, TRUE);
	gtk_widget_set_size_request (rcv_frame, -1, 380);

	alignment2 = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment2);
	gtk_container_add (GTK_CONTAINER (rcv_frame), alignment2);

	scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow2);
	gtk_container_add (GTK_CONTAINER (alignment2), scrolledwindow2);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2),  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_SHADOW_IN);
/*
	rcv_text = vte_terminal_new();
	vte_terminal_set_backspace_binding(VTE_TERMINAL(rcv_text),
				     VTE_ERASE_ASCII_BACKSPACE);
	gtk_widget_show (rcv_text);
	gtk_container_add (GTK_CONTAINER (scrolledwindow2), rcv_text);
	
	rcv_label = gtk_label_new (("Receive:"));
	gtk_widget_show (rcv_label);
	gtk_frame_set_label_widget (GTK_FRAME (rcv_frame), rcv_label);
	gtk_label_set_use_markup (GTK_LABEL (rcv_label), TRUE);

	send_frame = gtk_frame_new (NULL);
	gtk_widget_show (send_frame);
	//gtk_box_pack_start (GTK_BOX (right_vbox), send_frame, TRUE, TRUE, 0);
	gtk_paned_pack2 (GTK_PANED (right_vpaned), send_frame, FALSE, TRUE);

	alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment1);
	gtk_container_add (GTK_CONTAINER (send_frame), alignment1);

	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow1);
	gtk_container_add (GTK_CONTAINER (alignment1), scrolledwindow1);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1),  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_IN);

	send_text = gtk_text_view_new ();
	gtk_widget_show (send_text);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (send_text), GTK_WRAP_CHAR);
	gtk_container_add (GTK_CONTAINER (scrolledwindow1), send_text);
	
	send_label = gtk_label_new (("Send:"));
	gtk_widget_show (send_label);
	gtk_frame_set_label_widget (GTK_FRAME (send_frame), send_label);
	gtk_label_set_use_markup (GTK_LABEL (send_label), TRUE);
*/	
	return right_vbox;
}
	

void CreateFace (GtkWidget *Vbox)
{
	GtkWidget *Hbox;
	GtkWidget *Hbox1;
	GtkWidget *LeftSetBox;
	GtkWidget *RightTextBox;
	UartControl uc;
	
	Hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);	
    gtk_box_pack_start (GTK_BOX (Vbox), Hbox, FALSE, FALSE, 0);
   	LeftSetBox = CreateUartSetFace(Hbox,uc.ULC);
	RightTextBox = CreateUartTextFace(Hbox,uc.URC);
	
}
int main(int argc, char **argv)
{
	GtkWidget *MainWindow;
	GtkWidget *Vbox;
	UartControl uc;
	 
    gtk_init(&argc, &argv);
   
    MainWindow = CreateMainWindow();
    Vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);	
	gtk_container_add (GTK_CONTAINER (MainWindow), Vbox);
    uc.MainWindow = MainWindow;
	CreateFace(Vbox);
    gtk_widget_show_all(MainWindow);
    gtk_main ();  
    

}
