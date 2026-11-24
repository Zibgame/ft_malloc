ifeq ($(HOSTTYPE),)
HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME        := libft_malloc_$(HOSTTYPE).so
LINK        := libft_malloc.so
TEST_NAME   := test_malloc
TEST_SRC    := test/main.cpp

CXX         := c++
CXXFLAGS    := -Wall -Wextra -Werror -Wno-unused-parameter -fPIC
INCLUDES    := -Iinclude
CPPFLAGS    := $(INCLUDES) -MMD -MP
LDFLAGS     := -shared
LDLIBS      := -pthread

BUILD_DIR   := .build
SRCS        := $(shell find . -type f \( -name '*.c' -o -name '*.cpp' \) \
				-not -path './$(BUILD_DIR)/*' \
				-not -path './libft/*' \
				-not -path './test/*' \
				-not -path './tests/*' | sort)
OBJS        := $(patsubst ./%.c,$(BUILD_DIR)/%.o,$(filter %.c,$(SRCS))) \
				$(patsubst ./%.cpp,$(BUILD_DIR)/%.o,$(filter %.cpp,$(SRCS)))
DEPS        := $(OBJS:.o=.d)

all: $(LINK)

test: all $(TEST_NAME)
	LD_PRELOAD=$(CURDIR)/$(LINK) ./$(TEST_NAME)

$(TEST_NAME): $(TEST_SRC)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(TEST_SRC) -o $@

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
	rm -f $(NAME) $(LINK) $(TEST_NAME)

re: fclean all

bonus: all

-include $(DEPS)

.PHONY: all test clean fclean re bonus
