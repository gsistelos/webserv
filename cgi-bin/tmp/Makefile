NAME		=	minibash

SRCS		=	$(addprefix srcs/, main.c \
				$(addprefix builtin/, builtin_exit.c builtin_test.c get_builtin.c) \
				$(addprefix executor/, executor.c execve_cmd.c fork_exec.c set_pipes.c wait_pids.c) \
				$(addprefix parser/, expansor.c interpreter.c lexer.c parser.c) \
				$(addprefix signal/, setup_signals.c) \
				$(addprefix test/, print_cmds.c print_fd.c print_tokens.c) \
				$(addprefix utils/,	expand.c free_cmd.c free_token.c get_path.c new_cmd.c \
									new_token.c quotes_len.c skip_whitespaces.c) \
				)

OBJS		=	$(SRCS:.c=.o)

LIBGS_DIR	=	libgs

LIBGS		=	$(LIBGS_DIR)/libgs.a