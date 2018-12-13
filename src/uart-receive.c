#include "uart-receive.h"
#include "uart-share.h"

static void SwitchWriteFile(GtkWidget *Check,gpointer  data)
{
	UartControl *uc = (UartControl *) data;	
	if(gtk_toggle_button_get_active
      (GTK_TOGGLE_BUTTON(uc->ULC.CheckReWriteFile)) == FALSE)
    {   
		uc->Filefd = 0;
    }
    /*open function*/
    else
	{	
		uc->ChooseFile = SAVE; 
		LoadFile(uc);
    }	
}  
static void SwitchReceiveTime(GtkWidget *Check,gpointer  data)
{
	UartControl *uc = (UartControl *) data;	
	if(uc->ShowTime == 0)
		uc->ShowTime = 1;
	else
		uc->ShowTime = 0;
} 

static void SwitchReceiveHex(GtkWidget *Check,gpointer  data)
{
	UartControl *uc = (UartControl *) data;
	if(uc->ShowHex == 0)
		uc->ShowHex = 1;
	else
		uc->ShowHex = 0;		
} 
static gboolean
user_function (GtkLabel *label,
               gchar    *uri,
               gpointer  user_data)
{
	//UartControl *uc = (UartControl *) user_data;
    return TRUE;	
}   
static void ClearTerminalData (GtkLabel *label,
               			gchar    *uri,
               			gpointer  data)
{
	UartControl *uc = (UartControl *) data;
    if(uc->URC.ReveTerminal)
        vte_terminal_reset(VTE_TERMINAL(uc->URC.ReveTerminal), TRUE, TRUE);
	
}   
static gboolean
SetReceiveNotifyEvent (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data)
{
	UartControl *uc = (UartControl *) data;
    gtk_label_set_text(GTK_LABEL(uc->UDC.LabelState),
                      _("Receive Setting"));
    return TRUE;
}        
void ReceiveSet(GtkWidget *Hbox,UartControl *uc)
{
	GtkWidget *CheckReWriteFile;
	GtkWidget *CheckReTime;
	GtkWidget *CheckHex;
	GtkWidget *Hseparator;
	GtkWidget *Vpaned;
	GtkWidget *RcvFrame;
    GtkWidget *LabelSave;
    GtkWidget *LabelClear;
	GtkWidget *LabelSpace;
	GtkWidget *Table;
	
	Vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
	gtk_container_add (GTK_CONTAINER (Hbox), Vpaned);
	RcvFrame = gtk_frame_new (_("Receive Setting"));
	gtk_frame_set_label_align(GTK_FRAME(RcvFrame),0.5,0.3);
    SetWidgetStyle(RcvFrame,"black",12);
	gtk_paned_pack1 (GTK_PANED (Vpaned), RcvFrame, FALSE, TRUE);
	
    gtk_widget_add_events(Vpaned,GDK_ENTER_NOTIFY_MASK);  	
	g_signal_connect(G_OBJECT(Vpaned), 
                    "enter-notify-event", 
                     G_CALLBACK(SetReceiveNotifyEvent),
                     (gpointer) uc);
	
    Table = gtk_grid_new();
    gtk_container_add (GTK_CONTAINER (Vpaned), Table);
	gtk_grid_set_column_homogeneous(GTK_GRID(Table),TRUE);

	LabelSpace = gtk_label_new(" ");
	gtk_grid_attach(GTK_GRID(Table) ,LabelSpace , 0 , 0 , 2 , 1);
	
	CheckReWriteFile = gtk_check_button_new_with_label(_("Receive Write File"));
    uc->ULC.CheckReWriteFile = CheckReWriteFile;
	gtk_grid_attach(GTK_GRID(Table) , CheckReWriteFile , 0 , 1 , 2 , 1);
	g_signal_connect(G_OBJECT(CheckReWriteFile), 
                    "released", 
                    G_CALLBACK(SwitchWriteFile), 
                    (gpointer)uc);
	
	CheckReTime      = gtk_check_button_new_with_label(_("Display Receive time"));
    uc->ULC.CheckReTime = CheckReTime;
	gtk_grid_attach(GTK_GRID(Table) , CheckReTime , 0 , 2 , 2 , 1);
	g_signal_connect(G_OBJECT(CheckReTime), 
                    "released", 
                    G_CALLBACK(SwitchReceiveTime), 
                    (gpointer)uc);
	
	CheckHex         = gtk_check_button_new_with_label(_("Display Hex"));
    uc->ULC.CheckHex = CheckHex;
	gtk_grid_attach(GTK_GRID(Table) , CheckHex , 0 , 3 , 2 , 1);
	g_signal_connect(G_OBJECT(CheckHex), 
                    "released",
                    G_CALLBACK(SwitchReceiveHex), 
                    (gpointer)uc);
	
    LabelSave = gtk_label_new (_("<a href=\"null\">""<span color=\"#0266C8\">Save</span>""</a>"));
    gtk_label_set_use_markup (GTK_LABEL (LabelSave), TRUE); 
    gtk_grid_attach(GTK_GRID(Table) , LabelSave , 0 , 4, 1 , 1);
	g_signal_connect(G_OBJECT(LabelSave), 
                    "activate-link", 
                    G_CALLBACK(user_function), 
                    (gpointer) uc);     

    LabelClear = gtk_label_new (_("<a href=\"null\">""<span color=\"#0266C8\">Clear</span>""</a>"));
    gtk_label_set_use_markup (GTK_LABEL (LabelClear), TRUE); 
    gtk_grid_attach(GTK_GRID(Table) , LabelClear , 1 , 4, 1 , 1); 
	g_signal_connect(G_OBJECT(LabelClear), 
                    "activate-link", 
                    G_CALLBACK(ClearTerminalData), 
                    (gpointer) uc);

   	Hseparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(Table) , Hseparator , 0 , 5 , 2 , 1);
    
    gtk_grid_set_row_spacing(GTK_GRID(Table), 3);
    gtk_grid_set_column_spacing(GTK_GRID(Table), 3);

}
