# 1. 定义编译器和编译选项
CC = gcc
CFLAGS = -Wall -g   # -Wall 显示所有警告, -g 生成调试信息

# 2. 定义目标可执行文件的名称
TARGET = test_button

# 3. 列出所有的源文件
SRCS = main.c multi_button.c

# 4. 根据源文件生成目标文件列表 (.c 替换为 .o)
OBJS = $(SRCS:.c=.o)

# 5. 默认目标：编译可执行程序
all: $(TARGET)

# 6. 链接步骤：将 .o 文件链接成最终的可执行文件
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# 7. 编译步骤：将 .c 文件编译成 .o 文件
# % 是通配符，$< 表示依赖文件，$@ 表示目标文件
%.o: %.c multi_button.h
	$(CC) $(CFLAGS) -c $< -o $@

# 8. 清理命令：用于删除生成的文件
clean:
	rm -f $(OBJS) $(TARGET)

# 9. 运行命令：编译并立即运行
run: all
	./$(TARGET)