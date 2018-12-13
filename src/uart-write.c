#include "uart-write.h"
#include "uart-share.h"
int WriteUart(const char *WriteData,int WriteLen,UartControl *uc)
{
    int Ret;
    char *SendCountBuf = _("TX:");
    char Tmp[128] = { 0 };
    int Serialfd = uc->UP.fd;
    g_mutex_lock (&Mutex);
    Ret = write(Serialfd, WriteData,WriteLen);
    g_mutex_unlock (&Mutex);
    
    if(uc->AutoCleanSendData == 1)
    {
        CleanSendDate(uc);
    }
    else
    {
        uc->SendCount += 1;
        sprintf(Tmp,"%s    %d",SendCountBuf,uc->SendCount);
        gtk_label_set_text(GTK_LABEL(uc->UDC.LabelTx),Tmp);
    }        
    return Ret;

}
