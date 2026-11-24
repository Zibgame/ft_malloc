ifeq ($(HOSTTYPE),)
HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME        := libft_malloc_$(HOSTTYPE).so
LINK        := libft_malloc.so

CXX         := c++
CXXFLAGS    := -Wall -Wextra -Werror -Wno-unused-parameter -fPIC
CPPFLAGS    := -Iinclude -MMD -MP
LDFLAGS     := -shared
LDLIBS      := -pthread

BUILD_DIR   := .build
SRCS        := $(shell find . -type f \( -name '*.c' -o -name '*.cpp' \) \
				-not -path './$(BUILD_DIR)/*' \
				-not -path './libft/*' \
				-not -path './tests/*' | sort)
OBJS        := $(patsubst ./%.c,$(BUILD_DIR)/%.o,$(filter %.c,$(SRCS))) \
				$(patsubst ./%.cpp,$(BUILD_DIR)/%.o,$(filter %.cpp,$(SRCS)))
DEPS        := $(OBJS:.o=.d)

all: $(LINK)

$(LINK): $(NAME)
	ln -sfn $(NAME) $(LINK)

$(NAME): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -x c++ -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(NAME) $(LINK)

re: fclean all

bonus: all

-include $(DEPS)

.PHONY: all clean fclean re bonus
