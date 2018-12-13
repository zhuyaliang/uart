#include "uart-bottom.h"
#include "uart-write.h"
#include "uart-share.h"

static gpointer SendUseFileUart(gpointer data)
{
    UartControl *uc = (UartControl *) data;
    int WriteLen;
    char WriteData[100] = { 0 };

    gtk_widget_set_sensitive(uc->UDC.Button, FALSE);

    while(1)
    {
        if(uc->UseFilefd < 0 )
        {
            gtk_widget_set_sensitive(uc->UDC.Button,TRUE);
            if(uc->UseFilefd > 0)        
                close(uc->UseFilefd);
            uc->UseFilefd = -1;
            g_thread_exit(NULL);
        }
        WriteLen = read(uc->UseFilefd,WriteData,100);
        if(WriteLen <= 0)
        {
            gtk_widget_set_sensitive(uc->UDC.Button,TRUE);
            if(uc->UseFilefd > 0)
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
    return NULL;
}
static void SendData (GtkLabel *Button,gpointer  data)
{
	UartControl *uc = (UartControl *) data;
    int SendLen = 0;
    GtkTextIter Start,End;
    GtkTextBuffer *Buffer = NULL;
    gchar *text = NULL;

    if (uc->UP.fd < 0)
    {
        MessageReport(_("Send Data"),_("Send data fail, Serial don`t open"),ERROR);
        return;
    }
    if(uc->UseFilefd > 0 )
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
            printf("text = %s\r\n",text);
            WriteUart(text,SendLen,uc);
        }
        else
        {
            MessageReport(_("Send Data"),_("Send data can not be empty"),ERROR);
        }
    }
}
static gboolean
ReceiveCountNotifyEvent (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data)
{
	UartControl *uc = (UartControl *) data;
    gtk_label_set_text(GTK_LABEL(uc->UDC.LabelState),
                      _("Receive Count"));
    return TRUE;
}
static gboolean
SendCountNotifyEvent (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data)
{
	UartControl *uc = (UartControl *) data;
    gtk_label_set_text(GTK_LABEL(uc->UDC.LabelState),
                      _(" Send Count    "));
    return TRUE;
}
void CreateUartBottom(GtkWidget *Vbox,UartControl *uc)
{
    GtkWidget *image;
    GtkWidget *LabelState;
    GtkWidget *LabelRx;
    GtkWidget *LabelTx;

    GtkWidget *Button;
    GtkWidget *Hbox;

    Hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start (GTK_BOX (Vbox), Hbox, FALSE, FALSE, 0);

    image = gtk_image_new_from_icon_name ("audio-speakers",
                                           GTK_ICON_SIZE_MENU);
    gtk_box_pack_start (GTK_BOX (Hbox), image, TRUE, TRUE, 0);
    LabelState = gtk_label_new(_("Serial Assistant"));
    uc->UDC.LabelState = LabelState;
    gtk_box_pack_start (GTK_BOX (Hbox), LabelState, TRUE, TRUE, 0);

    LabelRx = gtk_label_new(_("RX:    0"));
    gtk_box_pack_start (GTK_BOX (Hbox), LabelRx, FALSE, FALSE, 0);
    SetWidgetStyle(LabelRx,"blue",11);
    uc->UDC.LabelRx = LabelRx;

    gtk_widget_add_events(LabelRx,GDK_ENTER_NOTIFY_MASK);
	g_signal_connect(LabelRx,
                    "enter-notify-event",
                     G_CALLBACK(ReceiveCountNotifyEvent),
                     (gpointer) uc);

    LabelTx = gtk_label_new(_("TX:    0"));
    gtk_box_pack_start (GTK_BOX (Hbox), LabelTx, TRUE, FALSE, 0);
    SetWidgetStyle(LabelTx,"blue",11);
    uc->UDC.LabelTx = LabelTx;

    gtk_widget_add_events(LabelTx,GDK_ENTER_NOTIFY_MASK);
	g_signal_connect(LabelTx,
                    "enter-notify-event",
                     G_CALLBACK(SendCountNotifyEvent),
                     (gpointer) uc);

    Button = gtk_button_new_with_label (_("Send"));
    SetWidgetStyle(Button,"black",11);
    gtk_box_pack_start (GTK_BOX (Hbox), Button, TRUE, FALSE, 0);
    uc->UDC.Button = Button;
    g_signal_connect(G_OBJECT(Button), "clicked", G_CALLBACK(SendData), uc);

}
