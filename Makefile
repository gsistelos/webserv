NAME		=	webserv

SRCS		=	$(addprefix srcs/,	Cgi.cpp Client.cpp Location.cpp Error.cpp HttpRequest.cpp \
									HttpResponse.cpp main.cpp Parser.cpp Server.cpp Fd.cpp \
									WebServ.cpp)

OBJS		=	$(SRCS:.cpp=.o)

CXX			=	g++

CXXFLAGS	=	-Wall -Werror -Wextra -std=c++98 -Iincludes

RM			=	rm -f

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(OBJS) -o $(NAME)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean:
	$(RM) $(OBJS) $(NAME)

re: fclean all

.PHONY: all clean fclean re
