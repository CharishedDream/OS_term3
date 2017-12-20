SOURCE= main.cpp server_network.cpp server_functions.cpp
MYPROGRAM = server
LIBS=pthread
CC=g++

all:$(MYPROGRAM)
$(MYPROGRAM) : $(SOURCE)
	$(CC) $(SOURCE) -o $(MYPROGRAM) -l$(LIBS) 

clean:
	rm -f $(MYPROGRAM) 

