#include "uart-share.h"

int MessageReport(const char *Title,const char *Msg,int nType)
{
    GtkWidget *dialog =NULL;
    int nRet;
    switch(nType)
    {
        case ERROR:
        {
            dialog = gtk_message_dialog_new(GTK_WINDOW(MainWindow),
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_MESSAGE_ERROR,
                                            GTK_BUTTONS_OK,
                                            "%s",Title);
            break;
        }
        case WARING:
        {
            dialog = gtk_message_dialog_new(GTK_WINDOW(MainWindow),
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_MESSAGE_WARNING,
                                            GTK_BUTTONS_OK,
                                            "%s",Title);
            break;
        }
        case INFOR:
        {
            dialog = gtk_message_dialog_new(GTK_WINDOW(MainWindow),
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_MESSAGE_INFO,
                                            GTK_BUTTONS_OK,
                                            "%s",Title);
            break;
        }
        case QUESTION:
        {
            dialog = gtk_message_dialog_new(GTK_WINDOW(MainWindow),
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_MESSAGE_QUESTION,
                                            GTK_BUTTONS_YES_NO,
                                            "%s",Title);
            break;
        }
        default :
            break;

    }
    gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog),TYPEMSG,Msg);
    gtk_window_set_title(GTK_WINDOW(dialog),_("Message"));
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
void SetLableFontType(GtkWidget *Lable ,
                      const char *Color,
                      int        FontSzie,
                      const char *Word)
{
    char LableTypeBuf[200] = { 0 };

    sprintf(LableTypeBuf,"<span foreground=\'%s\'weight=\'light\'font_desc=\'%d\'>%s</span>",Color,FontSzie,Word);
    gtk_label_set_markup(GTK_LABEL(Lable),LableTypeBuf); 

}

static int OpenFileName  (const char * FileName)
{
	int fd = -1;
	int Ret;

	if(access(FileName,F_OK) == -1 )
	{
		fd = open(FileName,O_RDWR|O_APPEND|O_CREAT,0777);
		if(fd < 0)
		{
			MessageReport(_("Create File"),_("Create File Fail"),ERROR);
		}
	}
	else if(access(FileName, W_OK) == -1)
	{
		MessageReport(_("Open File"),_("Open File Fail"),ERROR);
	}
    else
	{
		Ret = MessageReport(_("Open File"),_("Whether to clear the contents of the file"),QUESTION);
		if(Ret == GTK_RESPONSE_YES)
    	{
        	fd = open(FileName,O_WRONLY|O_APPEND|O_TRUNC);
        	if(fd < 0)
        	{
        		MessageReport(_("Create File"),_("Create File Fail"),ERROR);
        	}
    	}
    	else
    	{
        	fd = open(FileName,O_WRONLY|O_APPEND);
        	if(fd < 0)
        	{
        		MessageReport(_("Create File"),_("Create File Fail"),ERROR);
        	}
    	}
	}

	return fd;
}

static int OpenUseFile (const char *FileName)
{
    int fd = -1;

    if(access(FileName,R_OK) == -1)
	{
		MessageReport(_("Open File"),_("Open File Fail"),ERROR);
        return -1;
	}

    fd = open(FileName,O_RDONLY);
    if(fd < 0)
    {
        MessageReport(_("Create File"),_("Create File Fail"),ERROR);
    }
    return fd;
}

static void ChooseFileWrite (GtkButton *button, gpointer data)
{
    char *FileName = NULL;
    int fd;
    UartControl *uc = (UartControl *)data;

    FileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (uc->ChooseDialog));

    switch (uc->ChooseFile )
    {
        case SAVE:
           	fd = OpenFileName(FileName);
           	if(fd > 0 )
           	{
           		uc->Filefd = fd;
                uc->Redirect = 1;
           	}
            else
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(uc->ULC.CheckReWriteFile),FALSE);
                        
            break;
        case CHOOSE:
            fd = OpenUseFile(FileName);
            if(fd > 0)
            {
                uc->UseFilefd = fd;
                uc->UseFile = 1;
            }
            else
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(uc->ULC.CheckUseFile),FALSE);
            break;
        default:
            break;

    }
    gtk_widget_destroy(GTK_WIDGET(uc->ChooseDialog));
}
static void  CloseChooseFileWrite(GtkButton *button, gpointer data)
{
	UartControl *uc = (UartControl *)data;
    if(uc->ChooseFile == SAVE)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(uc->ULC.CheckReWriteFile),FALSE);
    else
       gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(uc->ULC.CheckUseFile),FALSE);
    gtk_widget_destroy(GTK_WIDGET(uc->ChooseDialog));
}

static GtkWidget* CreateFileChoose (UartControl *uc)
{
    GtkWidget *ButtonCancel;
    GtkWidget *ButtonOk;
	GtkWidget *FileChoose;
	
    if(uc->ChooseFile == SAVE) 
    {
        FileChoose = gtk_file_chooser_dialog_new (_("Choose Write File"), 
                                                     GTK_WINDOW(uc->MainWindow), 
                                                     GTK_FILE_CHOOSER_ACTION_SAVE, 
                                                     NULL,NULL);

        ButtonOk = gtk_button_new_with_label (_("save"));
    }
    else 
    {
    	FileChoose = gtk_file_chooser_dialog_new ("", 
                                                  GTK_WINDOW(uc->MainWindow), 
                                                  GTK_FILE_CHOOSER_ACTION_OPEN, 
                                                  NULL,NULL);

        ButtonOk = gtk_button_new_with_label (_("Choose"));	
    }
    
    uc->ChooseDialog = FileChoose;
    gtk_window_set_type_hint (GTK_WINDOW (FileChoose), GDK_WINDOW_TYPE_HINT_DIALOG);

    ButtonCancel = gtk_button_new_with_label (_("Cancel"));
    gtk_widget_show (ButtonCancel);
    gtk_dialog_add_action_widget (GTK_DIALOG (FileChoose), 
                                  ButtonCancel, 
                                  GTK_RESPONSE_CANCEL);
  
    gtk_widget_show (ButtonOk);
    gtk_dialog_add_action_widget (GTK_DIALOG (FileChoose), 
                                  ButtonOk, 
                                  GTK_RESPONSE_OK);
   
    g_signal_connect ((gpointer) ButtonCancel, "clicked",
            G_CALLBACK (CloseChooseFileWrite),uc);
    g_signal_connect ((gpointer) ButtonOk, "clicked",
            G_CALLBACK (ChooseFileWrite),uc);
    gtk_widget_set_can_default(ButtonOk,TRUE);
    return FileChoose;
}


int LoadFile (UartControl *uc)
{
    GtkWidget *WindowFileChoose;
     
    WindowFileChoose = (GtkWidget *)CreateFileChoose(uc);
    gtk_widget_show (WindowFileChoose);

    return 0;
}
void CleanSendDate(UartControl *uc)
{
    GtkTextBuffer *Buffer = NULL;

    Buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (uc->URC.SendText));
    gtk_text_buffer_set_text (GTK_TEXT_BUFFER(Buffer), (""), -1);
}
