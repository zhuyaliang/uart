#include "uart-send.h"
#include "uart-share.h"
#include "uart-write.h"

static gboolean AutoWriteUart (gpointer data)
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
    if(SendLen > 0)
    {
        SendLen += 1;     
        text[strlen(text)] = '\n';
        WriteUart(text,SendLen,uc);
    }

    return TRUE;
}        
static void SetAutoSend (UartControl *uc)
{
    const char *text;
    int CycleTime;
    int TimeId;
    
    text = gtk_entry_get_text(GTK_ENTRY(uc->ULC.EntrySendCycle));
    CycleTime = atoi(text);
    if(CycleTime == 0)
    {
        MessageReport(_("Auto Send Data"),_("Auto Send data fail,No setting time"),ERROR);
    } 
    else
    {        
        TimeId = g_timeout_add(CycleTime,(GSourceFunc)AutoWriteUart,uc); 
        uc->TimeId = TimeId;
    }
}        
static void RemoveAutoSend(UartControl *uc)
{
    if(uc->TimeId > 0)
        g_source_remove(uc->TimeId);
}        
static void SwitchAutoSend(GtkWidget *Check,gpointer  data)
{
	UartControl *uc = (UartControl *) data;

    if(uc->UP.fd < 0)
    {
        MessageReport(_("Auto Send Data"),
                      _("Auto Send data fail,No open serial"),ERROR);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(Check),
                                      FALSE);
        return;
    }        
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

static void SwitchAutoClean(GtkWidget *Check,gpointer  data)
{
	UartControl *uc = (UartControl *) data;
	if(uc->AutoCleanSendData == 0)
		uc->AutoCleanSendData = 1;
	else
		uc->AutoCleanSendData = 0;		
}

static void SwitchUseFile(GtkWidget *Check,gpointer  data)
{
	UartControl *uc = (UartControl *) data;
    if(gtk_toggle_button_get_active
      (GTK_TOGGLE_BUTTON(uc->ULC.CheckUseFile)) == TRUE)
    {
        uc->ChooseFile = CHOOSE; 
		LoadFile(uc);
        gtk_widget_set_sensitive(uc->ULC.CheckAutoSend, FALSE);
    }        
    else
    {
        if(uc->UseFilefd > 0)
        {
            close(uc->UseFilefd);
            uc->UseFilefd = -1;
        }    
        gtk_widget_set_sensitive(uc->ULC.CheckAutoSend, TRUE);
    }        
}


static void SendFile (GtkLabel *label,
               			gchar    *uri,
               			gpointer  user_data)
{
	//UartControl *uc = (UartControl *) user_data;
}
static void ClearSendData (GtkLabel *label,
               			gchar    *uri,
               			gpointer  user_data)
{
	UartControl *uc = (UartControl *) user_data;
    CleanSendDate(uc);
}
static gboolean
SetSendNotifyEvent (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data)
{
	UartControl *uc = (UartControl *) data;
    gtk_label_set_text(GTK_LABEL(uc->UDC.LabelState),
                      _("Send Setting"));
    return TRUE;
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
	SendFrame = gtk_frame_new (_("Send Setting"));
	gtk_frame_set_label_align(GTK_FRAME(SendFrame),0.5,0.3);
    SetWidgetStyle(SendFrame,"black",12);
	gtk_paned_pack1 (GTK_PANED (Vpaned), SendFrame, FALSE, TRUE);

    gtk_widget_add_events(Vpaned,GDK_ENTER_NOTIFY_MASK);
	g_signal_connect(G_OBJECT(Vpaned),
                    "enter-notify-event",
                     G_CALLBACK(SetSendNotifyEvent),
                     (gpointer) uc);
    
    Table = gtk_grid_new();
    gtk_container_add (GTK_CONTAINER (Vpaned), Table);
	gtk_grid_set_column_homogeneous(GTK_GRID(Table),TRUE);

	LabelSpace = gtk_label_new(" ");
	gtk_grid_attach(GTK_GRID(Table) ,LabelSpace , 0 , 0 , 2 , 1);

	CheckUseFile = gtk_check_button_new_with_label(_("Use File Data"));
	gtk_grid_attach(GTK_GRID(Table) , CheckUseFile , 0 , 1 , 2 , 1);
    uc->ULC.CheckUseFile = CheckUseFile;
	g_signal_connect(G_OBJECT(CheckUseFile),
                    "released",
                    G_CALLBACK(SwitchUseFile),
                    (gpointer) uc);

	CheckAutoClean      = gtk_check_button_new_with_label(_("Auto Empty"));
    uc->ULC.CheckAutoClean = CheckAutoClean;
    gtk_grid_attach(GTK_GRID(Table) , CheckAutoClean , 0 , 2 ,2 , 1);
	g_signal_connect(G_OBJECT(CheckAutoClean),
                    "released",
                    G_CALLBACK(SwitchAutoClean),
                    (gpointer) uc);

	CheckAutoSend        = gtk_check_button_new_with_label(_("Auto Send Cycle"));
    uc->ULC.CheckAutoSend = CheckAutoSend;
    gtk_grid_attach(GTK_GRID(Table) , CheckAutoSend , 0 , 3, 2 , 1);
    g_signal_connect(G_OBJECT(CheckAutoSend),
                    "released",
                    G_CALLBACK(SwitchAutoSend),
                    (gpointer) uc);

    EntrySendCycle = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(EntrySendCycle),6);
    gtk_entry_set_text(GTK_ENTRY(EntrySendCycle),"1000");
    gtk_widget_set_hexpand (EntrySendCycle,TRUE);
    gtk_widget_set_halign (EntrySendCycle,GTK_ALIGN_CENTER);
    gtk_widget_set_tooltip_text(EntrySendCycle,_("Automatic transmission cycle, unit milliseconds"));
    gtk_entry_set_placeholder_text(GTK_ENTRY(EntrySendCycle),_("ms"));
    uc->ULC.EntrySendCycle = EntrySendCycle;
    gtk_grid_attach(GTK_GRID(Table) , EntrySendCycle , 0 , 4, 2 , 1);

    LabelSendFile= gtk_label_new (_("<a href=\"null\">""<span color=\"#0266C8\">Send File</span>""</a>"));
    gtk_label_set_use_markup (GTK_LABEL (LabelSendFile), TRUE);
    g_signal_connect(G_OBJECT(LabelSendFile), "activate-link", G_CALLBACK(SendFile), (gpointer) uc);
    gtk_grid_attach(GTK_GRID(Table) , LabelSendFile , 0 , 5, 1 , 1);

    LabelClear= gtk_label_new (_("<a href=\"null\">""<span color=\"#0266C8\">Clear</span>""</a>"));
    gtk_label_set_use_markup (GTK_LABEL (LabelClear), TRUE);
    g_signal_connect(G_OBJECT(LabelClear), "activate-link", G_CALLBACK(ClearSendData), (gpointer) uc);
    gtk_grid_attach(GTK_GRID(Table) , LabelClear , 1 , 5, 1 , 1);

   	Hseparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(Table) , Hseparator , 0 , 6 , 2 , 1);

    gtk_grid_set_row_spacing(GTK_GRID(Table), 3);
    gtk_grid_set_column_spacing(GTK_GRID(Table), 3);

}
