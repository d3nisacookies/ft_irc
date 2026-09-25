CXX      := c++

CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -g
INCLUDES := -Iincludes

TARGET    := ircserv
SRC_DIR   := srcs
BUILD_DIR := build

#replace main.cpp after testing
SRCS      := $(SRC_DIR)/main.cpp \
             $(SRC_DIR)/Server.cpp \
             $(SRC_DIR)/Client.cpp \
             $(SRC_DIR)/Channel.cpp \
             $(SRC_DIR)/IRCMessage.cpp

OBJS      := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

DEPS      := $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(TARGET)

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re
