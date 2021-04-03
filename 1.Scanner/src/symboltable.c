#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<math.h>
#include<stdbool.h>
#include"header.h"

#define TABLE_SIZE	512

symtab * hash_table[TABLE_SIZE];
extern int linenumber;
ID * idList = NULL;

int HASH(char * str){
  int idx=0;
  while(*str){
    idx = idx << 1;
    idx+=*str;
    str++;
  }	
  return (idx & (TABLE_SIZE-1));
}

/*returns the symbol table entry if found else NULL*/

symtab * lookup(char *name){
  int hash_key;
  symtab* symptr;
  if(!name)
    return NULL;
  hash_key=HASH(name);
  symptr=hash_table[hash_key];

  while(symptr){
    if(!(strcmp(name,symptr->lexeme)))
      return symptr;
    symptr=symptr->front;
  }
  return NULL;
}


void insertID(char *name){
  int hash_key;
  symtab* ptr;
  symtab* symptr=(symtab*)malloc(sizeof(symtab));	
  
  hash_key=HASH(name);
  ptr=hash_table[hash_key];
  
  if(ptr==NULL){
    /*first entry for this hash_key*/
    hash_table[hash_key]=symptr;
    symptr->front=NULL;
    symptr->back=symptr;
  }
  else{
    symptr->front=ptr;
    ptr->back=symptr;
    symptr->back=symptr;
    hash_table[hash_key]=symptr;	
  }
  
  strcpy(symptr->lexeme,name);
  symptr->line=linenumber;
  symptr->counter=1;
}

void printSym(symtab* ptr) 
{
      printf(" Name = %s \n", ptr->lexeme);
      printf(" References = %d \n", ptr->counter);
}

void printSymTab()
{
    int i;
    printf("----- Symbol Table ---------\n");
    for (i=0; i<TABLE_SIZE; i++)
    {
        symtab* symptr;
  symptr = hash_table[i];
  while (symptr != NULL)
  {
            printf("====>  index = %d \n", i);
      printSym(symptr);
      symptr=symptr->front;
  }
    }
}

ID *insertionSort(ID *ptr){
  ID *last;
  while(ptr!=NULL){
    ID *tmp = ptr;
    ID *cmp = tmp->prev;
    last = ptr;
    ptr = ptr->next;
    while(cmp!=NULL && strcmp(cmp->name, tmp->name)>0){
      tmp->prev = cmp->prev;
      if(tmp->prev!=NULL)	tmp->prev->next = tmp;
      
      cmp->next = tmp->next;
      if(cmp->next!=NULL)	cmp->next->prev = cmp;
      
      tmp->next = cmp;
      cmp->prev = tmp;
      
      cmp = tmp->prev;
    }
  }
  while(last!=NULL){
    ptr = last;
    last = last->prev;
  }
  return ptr;
}

void printIdList(ID *ptr){
  while(ptr!=NULL){
    printf("%-32s%3d\n", ptr->name, ptr->freq);
    ptr = ptr->next;
  }
}

void printIdListNew (symtab **id_arr, int num_ids) {
  for (int i = 0; i < num_ids; i++) {
    printf("%-32s%3d\n", id_arr[i]->lexeme, id_arr[i]->counter);
  }
}

bool isReserved(char *s){
  static char reservedWords[9][9] = {"return", "typedef", "if", "else", "int", "float", "for", "void", "while"};
  for(int i = 0; i < 9; i++)
    if(strcmp(s, reservedWords[i]) == 0)
      return true;
  return false;
}

void printFreqOfId(){
    printf("\nFrequency of identifiers:\n");
    for(int i = 0; i < TABLE_SIZE; i++){
      symtab* symptr;
      symptr = hash_table[i];
      while (symptr != NULL){
        if(!isReserved(symptr->lexeme)){
          ID* tmptr = (ID*)malloc(sizeof(ID));
          strcpy(tmptr->name, symptr->lexeme);
          tmptr->freq = symptr->counter;
          tmptr->prev = NULL;
          tmptr->next = idList;

          if(idList!=NULL)	idList->prev = tmptr;
          idList = tmptr;
        }
        symptr=symptr->front;
      }
    }
  idList = insertionSort(idList);
  printIdList(idList);
}

int idCmp (const void *a, const void *b) {
  symtab *ptr_a = *(symtab **)a;
  symtab *ptr_b = *(symtab **)b;
  return strcmp(ptr_a->lexeme, ptr_b->lexeme);
}

void printFreqOfIdNew () {
  int num_ids = 0;

  printf("\nFrequency of identifiers:\n");

  // traverse 1st time to get total # of IDs
  for (int i = 0; i < TABLE_SIZE; i++) {
    symtab *symptr = hash_table[i];

    while (symptr != NULL) {
      if (!isReserved(symptr->lexeme)) {
        num_ids ++;
        // printf("found ID: %s\n", symptr->lexeme);
      } else {
        // printf("found reserved: %s | ", symptr->lexeme);
        strcpy(symptr->lexeme, "RESERVED\0");
        // printf("after change: %s\n", symptr->lexeme); 
      }
      symptr = symptr->front;
    }
  }

  symtab **id_arr = (symtab **)malloc(sizeof(symtab *) * num_ids);
  // traverse 2nd time to fill in the pointer array
  int arr_cur_idx = 0;
  for (int i = 0; i < TABLE_SIZE; i++) {
    symtab *symptr = hash_table[i];
    while (symptr != NULL) {
      if ( strcmp(symptr->lexeme, "RESERVED") != 0 ) {
        id_arr[arr_cur_idx ++] = symptr;
      }
      symptr = symptr->front;
    }
  }

  qsort(id_arr, num_ids, sizeof(symtab *), idCmp);
  printIdListNew(id_arr, num_ids);
}