#include "uart-write.h"
#include "uart-share.h"
int WriteUart(const char *WriteData,int WriteLen,UartControl *uc)
{
    int Ret;
    char *SendCountBuf = _("TX:");
    char Tmp[128] = { 0 };

    g_mutex_lock (&Mutex);
    while(CurrentData == 0) //防止意外唤醒
    {
        g_cond_wait (&Cond, &Mutex);
    }
    CurrentData = 0;
    Ret = write(Serialfd, WriteData,WriteLen);
    g_mutex_unlock (&Mutex);
    if (Ret < 0)
    {
    //    MessageReport(_("Send Data"),_("Send data fail,Send length is 0"),ERROR);
    }
    if(uc->AutoCleanSendData == 1)
    {
        CleanSendDate(uc);
    }
    else
    {
        uc->SendCount += 1;
        sprintf(Tmp,"%s    %d",SendCountBuf,uc->SendCount);
        gtk_button_set_label(GTK_BUTTON(uc->UDC.LabelTx),Tmp);
    }        
    return Ret;

}
