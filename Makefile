NAME     = webserv

SRCS     = $(addprefix srcs/, Cgi.cpp Client.cpp ConfigBlock.cpp Error.cpp Fd.cpp \
                              HttpRequest.cpp HttpResponse.cpp Location.cpp main.cpp \
                              Parser.cpp Server.cpp WebServ.cpp)

OBJS     = $(SRCS:.cpp=.o)

CXX      = g++

CXXFLAGS = -Wall -Werror -Wextra -std=c++98 -Iincludes

RM       = rm -f

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
