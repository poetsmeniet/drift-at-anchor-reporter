NAME = test_client

CODE_FILES = main.c tcp_client.c irclib.c config.c generic_unix_tools.c asciiHashMap.c

DOC_FILES = docs
DEBUG = YES
ifeq ($(DEBUG),YES)
	D = -g
else
	D =
endif

.PHONY: all clean docs

all: $(CODE_FILES)
	gcc -lm -Wno-parentheses -Wextra -fsanitize=undefined -Wall -g -o $(NAME) $(CODE_FILES)

#docs: Doxyfile
#	doxygen Doxyfile

#Doxyfile:
#	doxygen -g

clean:
	rm -rf $(NAME) $(DOC_FILES) *.o
