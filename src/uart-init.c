#include "uart-init.h"
#ifndef CMSPAR
#define CMSPAR   010000000000
#endif
void SetDefaultSerial(UartControl *uc)
{
	uc->UP.UartBaud    = 9600;
	uc->UP.UartParity  = 0;
	uc->UP.UartStop    = 1;
	uc->UP.UartData    = 8;
	uc->UP.fd          = -1;
	uc->ShowHex        = 0;
	uc->Filefd         = -1;
    uc->UseFilefd      = -1;
    uc->ShowTime       = 0;
    uc->AutoCleanSendData = 0; 
    uc->AutoSend       = 0;
    uc->TimeId         = -1;
    uc->SendCount      = 0;
    uc->ReceCount      = 0;
    uc->Portlist       = NULL;
}

int InitSerial(UartControl *uc)
{
	int fd;
	struct termios termios_old, termios_new;
    
    printf("uc->UP.UartPort = %s\r\n",uc->UP.UartPort);
	fd = open(uc->UP.UartPort, O_RDWR | O_NOCTTY | O_NDELAY);
	if(fd < 0)
	{
		return -1;
	}
    
	if(fcntl(fd, F_SETFL, 0) < 0)		//擦除以前的标志位，恢复成默认设置
	{
		return -1;
	}

	if(isatty(STDIN_FILENO) == 0)		//判断是否为终端设备，若为终端设备值为1，否则执行下面的打印
	{
		return -1;
	}


	if(tcgetattr(fd, &termios_old) != 0)
	{
		return -1;
	}

	bzero(&termios_new, sizeof(termios_new));

	termios_new.c_cflag |= CLOCAL;
	termios_new.c_cflag |= CREAD;
	termios_new.c_cflag &= ~CSIZE;

	switch(uc->UP.UartData)
	{
		case 5:
			termios_new.c_cflag |= CS5;
			break;
		case 6:
			termios_new.c_cflag |= CS6;
			break;
		case 7:
			termios_new.c_cflag |= CS7;
			break;
		case 8:
			termios_new.c_cflag |= CS8;
			break;
		default:
			termios_new.c_cflag |= CS8;
			break;
	}

	switch(uc->UP.UartParity)
	{
		case 0:		//no parity check
			termios_new.c_cflag &= ~PARENB;
			break;
		case 1:		//odd check
			termios_new.c_cflag |= PARENB;
			termios_new.c_cflag |= PARODD;
			termios_new.c_cflag |= (INPCK | ISTRIP);
			break;
		case 2:		//even check
			termios_new.c_cflag |= (INPCK | ISTRIP);
			termios_new.c_cflag |= PARENB;
			termios_new.c_cflag &= ~PARODD;
			break;
		case 3:
			termios_new.c_cflag |= PARENB | CMSPAR |PARODD;
			break;
		case 4:
			termios_new.c_cflag |= PARENB | CS8 | CMSPAR;
            break;
		default:			//no parity check
			termios_new.c_cflag &= ~PARENB;
			break;
	}

	switch(uc->UP.UartBaud)
	{
		case 1200:
			cfsetispeed(&termios_new, B1200);
			cfsetospeed(&termios_new, B1200);
			break;
		case 2400:
			cfsetispeed(&termios_new, B2400);
			cfsetospeed(&termios_new, B2400);
			break;
		case 4800:
			cfsetispeed(&termios_new, B4800);
			cfsetospeed(&termios_new, B4800);
			break;
		case 9600:
			cfsetispeed(&termios_new, B9600);
			cfsetospeed(&termios_new, B9600);
			break;
		case 19200:
			cfsetispeed(&termios_new, B19200);
			cfsetospeed(&termios_new, B19200);
			break;
		case 38400:
			cfsetispeed(&termios_new, B38400);
			cfsetospeed(&termios_new, B38400);
			break;
		case 57600:
			cfsetispeed(&termios_new, B57600);
			cfsetospeed(&termios_new, B57600);
			break;
		case 115200:
			cfsetispeed(&termios_new, B115200);
			cfsetospeed(&termios_new, B115200);
			break;
		case 230400:
			cfsetispeed(&termios_new, B230400);
			cfsetospeed(&termios_new, B230400);
			break;
		default:
			cfsetispeed(&termios_new, B9600);
			cfsetospeed(&termios_new, B9600);
			break;
	}

	if(uc->UP.UartStop == 1)
		termios_new.c_cflag &= ~CSTOPB;
	else if(uc->UP.UartStop == 2)
		termios_new.c_cflag |= CSTOPB;
	else
		termios_new.c_cflag &= ~CSTOPB;

	termios_new.c_cc[VMIN]  = 0;
	termios_new.c_cc[VTIME] = 0;
	tcflush(fd, TCIFLUSH);
	if(tcsetattr(fd, TCSANOW, &termios_new) != 0)
	{

		return -1;
	}

	return fd;
}
