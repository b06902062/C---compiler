struct symtab{
	char lexeme[32];
	struct symtab *front;
	struct symtab *back;
	int line;
	int counter;
};

struct idFreqToken{
	char name[32];
	int freq;
	struct idFreqToken *prev;
	struct idFreqToken *next;
};

typedef struct symtab symtab;
typedef struct idFreqToken ID;
symtab * lookup(char *name);
void insert(char *name);
