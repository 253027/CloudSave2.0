# 根目录的路径
ROOT_DIR := ..

# 子目录的源文件
SRC := $(wildcard *.cpp)

# 设置对象文件的输出目录
OBJ_DIR := $(ROOT_DIR)/build/FileServer
$(shell mkdir -p $(OBJ_DIR))

# 构造对象文件路径（保留子目录结构）
OBJ := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC))

# 使用 src 的对象文件
SDK_OBJ := $(wildcard $(ROOT_DIR)/build/src/*.o)

# 编译器和编译选项
CC := g++
CXXFLAGS ?= -O2 -MMD -MP # 添加依赖文件生成标志
LDFLAGS := -L$(ROOT_DIR)/lib
LDLIBS := -lcrypt -lmysqlclient -lspdlog

# 生成可执行文件
server: $(OBJ) $(SDK_OBJ)
	$(CC) $^ $(CXXFLAGS) -o $@ $(LDFLAGS) $(LDLIBS)

# 模式规则，从%.cpp生成%.o，生成依赖文件
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) -c $(CXXFLAGS) $< -o $@

# 自动包含生成的依赖文件 (.d)
-include $(OBJ:.o=.d)

# 清理规则，删除生成的二进制文件和对象文件及依赖文件
.PHONY: clean
clean:
	$(RM) $(OBJ_DIR)/*.o $(OBJ_DIR)/*.d server
