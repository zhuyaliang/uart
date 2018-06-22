#include "uart-init.h"
#include "uart-config.h"
#include "uart-share.h"
#include "uart-text.h"
#include "uart-receive.h"
#include "uart-send.h"
#include "uart-bottom.h"

GtkWidget *CreateMainWindow(void)
{
	GtkWidget *WindowMain;
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    
    WindowMain = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect_swapped(G_OBJECT(WindowMain), "destroy", G_CALLBACK(gtk_main_quit), NULL);  
    gtk_window_set_title(GTK_WINDOW(WindowMain),_("Serial port assistant"));
	gtk_window_set_position(GTK_WINDOW(WindowMain), GTK_WIN_POS_CENTER);
	gtk_widget_realize(WindowMain);
    gtk_container_set_border_width(GTK_CONTAINER(WindowMain),10);
	gtk_widget_set_size_request(WindowMain, 500, 500);
    
	return WindowMain;
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
       
void CreateFace (GtkWidget *Vbox,UartControl *uc)
{
	GtkWidget *Hbox;
	
	Hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);	
    gtk_box_pack_start (GTK_BOX (Vbox), Hbox, FALSE, FALSE, 0);
    
   	CreateUartSetFace(Hbox,uc);
	CreateUartTextFace(Hbox,uc);
    CreateUartBottom(Vbox,uc);
	
}

int main(int argc, char **argv)
{
	GtkWidget *Vbox;
	UartControl uc;
	 
    bindtextdomain (PACKAGE, LOCALEDIR); //关联包名称及其对际化翻译语言所在路径  
    textdomain (PACKAGE); 

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
