CXX = g++
CXXFLAGS = -Wall -I/usr/include/GL
LDFLAGS = -L/usr/X11R6/lib -L/usr/lib -lglut -lGL -lGLU -lX11 -lm -lXext -lXmu -lXi

TARGET = game
SRCS = game.cpp
OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean

