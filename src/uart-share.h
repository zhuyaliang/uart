#ifndef __UART_SHARE_H__
#define __UART_SHARE_H__

#include "uart.h"

#define     TYPEMSG    "<span foreground='red'font_desc='13'>%s </span>"
#define     ERROR      1
#define     WARING     2
#define     INFOR      3
#define     QUESTION   4
#define     SAVE       0
#define     CHOOSE     1

int MessageReport(const char *Title,const char *Msg,int nType);
void SetWidgetStyle(GtkWidget * Widget,const char *Color,int FontSize);
void SetLableFontType(GtkWidget  *Lable ,
                      const char *Color,
                      int         FontSzie,
                      const char *Word);
int LoadFile (UartControl *uc);
void CleanSendDate(UartControl *uc);
#endif
