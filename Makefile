CC = gcc  
MAINC = main.c
EXEC = style 

CFLAGS = `pkg-config --cflags --libs gtk+-3.0`
main:  
	$(CC)  $(MAINC)  -o $(EXEC) $(CFLAGS) -lm -lvte
clean:
	rm $(EXEC) -rf
