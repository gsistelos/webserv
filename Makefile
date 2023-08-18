NAME		=	webserv

SRCS		=	$(addprefix srcs/, Error.cpp main.cpp Parser.cpp WebServ.cpp Server.cpp Client.cpp)

OBJS		=	$(SRCS:.cpp=.o)

FLAGS		=

CXX			=	g++

CXXFLAGS	=	-Wall -Werror -Wextra -std=c++98

INCLUDE		=	-Iincludes

RM			=	rm -f

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(FLAGS) $(OBJS) -o $(NAME)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean:
	$(RM) $(OBJS) $(NAME)

re: fclean all

.PHONY: all clean fclean re
