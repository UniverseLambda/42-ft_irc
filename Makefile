.PHONY:	all re clean fclean distclean test FORCE
.SUFFIX: .cpp .o

NAME :=	ircserv

BUILD_SRCS := src/main.cpp
TEST_SRCS := test/test.cpp

SRCS =

include src/api/Include.mk
include src/emma/Include.mk
include src/internal/Include.mk
include src/data/Include.mk
include src/util/Include.mk

INCS :=	inc/api/IComm.hpp \
	inc/api/Interface.hpp \
	inc/data/Channel.hpp \
	inc/data/Forward.hpp \
	inc/data/User.hpp \
	inc/emma/Msg_manager.hpp \
	inc/emma/Parsing.hpp \
	inc/internal/Forward.hpp \
	inc/internal/Message.hpp \
	inc/internal/Origin.hpp \
	inc/internal/Server.hpp \
	inc/util/Optional.hpp \
	inc/util/Util.hpp \

SRCS_DIR := src
OBJS_DIR := .objs

CXX :=	c++
LD := $(CXX)
RM := rm
MKDIR := mkdir

CXX_FLAGS := -g3 -Wall -Wextra -Werror -std=c++98 -Iinc -fsanitize=address #-fsanitize=undefined
LD_FLAGS := -g3 -Wall -Wextra -Werror -std=c++98 -fsanitize=address #-fsanitize=undefined

OBJS :=	$(addprefix $(OBJS_DIR)/, $(SRCS:.cpp=.o))

BUILD_OBJS :=  $(addprefix $(OBJS_DIR)/, $(BUILD_SRCS:.cpp=.o))
TEST_OBJS := $(addprefix $(OBJS_DIR)/, $(TEST_SRCS:.cpp=.o))


all:		$(NAME)

$(NAME):	$(OBJS) $(BUILD_OBJS)
	$(LD) $(LD_FLAGS) -o $@ $(OBJS)  $(BUILD_OBJS)

$(OBJS_DIR)/%.o: %.cpp Makefile $(INCS)
	@$(MKDIR) -p $(dir $@)
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

clean:
	$(RM) -rf $(OBJS_DIR)

fclean:		clean
	$(RM) -f ircserv

distclean:	fclean

re: distclean all

compile_test: FORCE $(OBJS) $(TEST_OBJS)
	$(LD) $(LD_FLAGS) -o $(OBJS_DIR)/test_exec $(OBJS) $(TEST_OBJS)

test: compile_test
	$(OBJS_DIR)/test_exec

	$(RM) $(OBJS_DIR)/test_exec
