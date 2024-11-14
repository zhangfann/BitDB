CXX = g++ 
AFLAGS += -r

# 编译选项
#CXXFLAGS += -g -std=c++11 `mysql_config --cflags --libs`
CXXFLAGS += -g -std=c++14

# 头文件
CPPFLAGS += -I. -I/usr/include/jsoncpp \
	-I/opt/homebrew/Cellar/jsoncpp/1.9.4_1/include \
	-I../Include \
	-I/opt/homebrew/Cellar/openssl@1.1/1.1.1o/include \
	-I/opt/homebrew/opt/openssl/include

# 库
LDLIBS= -lpthread -ljsoncpp -lssl -lcrypto -l sqlite3
# 库目录
LDFLAGS +=  -L/opt/homebrew/Cellar/jsoncpp/1.9.4_1/lib/ \
	-L/opt/homebrew/Cellar/openssl@1.1/1.1.1o/lib \
	-L/opt/homebrew/opt/openssl/lib \
	$(LDLIBS)

TARGET_LIB= libsqlengine.a
LIB_OBJS= Bitcask.o \
	Statement.o \
	Catalog.o \
	Binder.o 

#all: main $(TARGET_LIB)
#all: main
all: $(TARGET_LIB)


# 静态库
$(TARGET_LIB): $(LIB_OBJS)
	$(AR) $(AFLAGS) $@ $^ 

#main: main.o $(LIB_OBJS)
#	$(CXX) $^ $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ 
.c.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $^ -o $@
.cpp.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $^ -o $@

clean:
	rm -f *.o main libdb.a
