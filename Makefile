CC = g++

TARGET = ipkcpc

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) -g -Wall -o $(TARGET) $(TARGET).cpp

clean:
	rm $(TARGET)