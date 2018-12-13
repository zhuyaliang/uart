#include "uart-read.h"
#include "uart-share.h"

static void AddReceiveTime(UartControl *uc)
{
	struct tm *t;
    char TimeBuf[128] = { 0 };
    time_t tt;
    time(&tt);
    t = localtime(&tt);
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
static int ConversionHex(char *ReadBuf,int size,UartControl *uc)
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
            uc->ReceCount += 1;
        }
    }
    return 0;
}
static void AlignData(char *ReadBuf,int size,UartControl *uc)
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
    for (pos = size; pos > 0; pos--) 
    {
        in_buffer--;
        if(*in_buffer == '\r' && *(in_buffer+1) != '\n')
        {
            g_string_insert_c(buffer_tmp, pos, '\n');
            size += 1;
        }
        if(*in_buffer == '\n' && *(in_buffer-1) != '\r')
        {
            g_string_insert_c(buffer_tmp, pos-1, '\r');
            AddTime = 1;
            size += 1;
        }
        if(*in_buffer == '\n' && *(in_buffer-1) == '\r')
        {
            AddTime = 1;
        }
    }
    vte_terminal_feed(VTE_TERMINAL(uc->URC.ReveTerminal), buffer_tmp->str, size);
    uc->ReceCount += 1;
}
static int WriteDataToFile(char *ReadBuf,int size,UartControl *uc)
{
	int pos;
    GString *buffer_tmp;
    gchar *in_buffer;

    buffer_tmp =  g_string_new(ReadBuf);
    in_buffer = buffer_tmp->str;
    in_buffer += size;

    for (pos = size; pos > 0; pos--) 
    {
        in_buffer--;
        if(*in_buffer =='\r' && *(in_buffer+1) != '\n')
        {
            g_string_insert_c(buffer_tmp, pos, '\n');
            size += 1;
        }
        if(*in_buffer =='\n' && *(in_buffer-1) != '\r')
        {
            g_string_insert_c(buffer_tmp, pos-1, '\r');
            size += 1;
        }
    }
	write(uc->Filefd,buffer_tmp->str, size);
    uc->ReceCount += 1;

    return 0;
}
static int DataHandle(char *ReadBuf,UartControl *uc)
{
    /*Data Write to File*/
	if(uc->Filefd > 0)
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
    return 0;
}
static gpointer ReadUart(gpointer data)
{
	UartControl *uc = (UartControl *) data;
	int ReadLen;
	fd_set rd;
    char ReadBuf[1024] = {0};
    struct timeval Timeout={0,0};
    int Serialfd = uc->UP.fd;
    while(1)
    {
    	FD_ZERO(&rd);
  		FD_SET(Serialfd,&rd);
  		while(FD_ISSET(Serialfd,&rd))
  		{
    	    if(ThreadEnd == 1)
                g_thread_exit(NULL);
            if(select(Serialfd+1,&rd,NULL,NULL,&Timeout) < 0)
      			MessageReport(_("Read Serial"),_("Read Serial Fail"),ERROR);
    		else
    		{
                usleep(200000);
      			g_mutex_lock(&Mutex);
				ReadLen = read(Serialfd, ReadBuf, 1024);
				g_mutex_unlock(&Mutex);
				DataHandle(ReadBuf,uc);
                memset(ReadBuf,'\0',ReadLen);
      		}
    	}
  	}
    return NULL;
}
gpointer CreateReadUart(UartControl *uc)
{
	return g_thread_new("ReadUart",(GThreadFunc)ReadUart,(gpointer)uc);
}

