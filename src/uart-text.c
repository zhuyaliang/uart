#include "uart-text.h"
static gboolean
ReceiveDataNotifyEvent (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data)
{
	UartControl *uc = (UartControl *) data;
    gtk_label_set_text(GTK_LABEL(uc->UDC.LabelState),
                      _("Receive Data"));
}
void CreateReceiveFace(GtkWidget *Vpaned,UartControl *uc)
{
    GtkWidget *ReveTerminal;
	GtkWidget *Frame;
	GtkWidget *Scrolled;

	Frame = gtk_frame_new (_("Receive Data From Serial"));
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

    gtk_widget_add_events(Scrolled,GDK_ENTER_NOTIFY_MASK);
	g_signal_connect(G_OBJECT(Scrolled),
                    "enter-notify-event",
                     G_CALLBACK(ReceiveDataNotifyEvent),
                     (gpointer) uc);

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

static gboolean
SendDataNotifyEvent (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data)
{
	UartControl *uc = (UartControl *) data;
    gtk_label_set_text(GTK_LABEL(uc->UDC.LabelState),
                      _("Send Data"));
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

	Frame = gtk_frame_new (_("Send Data Serial"));
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
    gtk_widget_add_events(Scrolled,GDK_ENTER_NOTIFY_MASK);
	g_signal_connect(G_OBJECT(Scrolled),
                    "enter-notify-event",
                     G_CALLBACK(SendDataNotifyEvent),
                     (gpointer) uc);

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

