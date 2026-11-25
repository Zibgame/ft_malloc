ifeq ($(HOSTTYPE),)
HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME        := libft_malloc_$(HOSTTYPE).so
LINK        := libft_malloc.so
TEST_NAME   := test_malloc
TEST_SRC    := test/main.cpp

CXX         := c++
CXXFLAGS    := -Wall -Wextra -Werror -std=c++17 -fPIC
INCLUDES    := -Iinclude
CPPFLAGS    := $(INCLUDES) -MMD -MP
LDFLAGS     := -shared
LDLIBS      := -pthread

BUILD_DIR   := .build
SRCS        := $(sort $(wildcard src/*.cpp))
OBJS        := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS        := $(OBJS:.o=.d)

all: $(LINK)

test: $(TEST_NAME)
	./$(TEST_NAME)

$(TEST_NAME): $(TEST_SRC) $(LINK) include/malloc.h
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(TEST_SRC) -L. -lft_malloc \
		-Wl,-rpath,'$$ORIGIN' $(LDLIBS) -o $@

$(LINK): $(NAME)
	ln -sfn $(NAME) $(LINK)

$(NAME): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

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
