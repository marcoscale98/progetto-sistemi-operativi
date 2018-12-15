#include <stdio.h>
#include <string.h>
#include "header/error.h"

#define BUF_SIZE 32

void divide(char* str, char* str_left, char* str_right, char delimiter) {
   //divide la str in due parti delimitate da un carattere
   int size;
   char* del;
   
   //per la stringa di sinistra
   size=strcspn(str, &delimiter); //restituisce la lunghezza della sottostringa iniziale che non contiene il delimitatore
   TEST_ERROR
   str_left = malloc((sizeof(char))*(size+1));
   TEST_ERROR
   strncpy(str_left, str, size);
   TEST_ERROR
   str_left[size]='\0';
   
   //per la stringa di destra
   del = strchr(str, delimiter);
   TEST_ERROR
   str_right = malloc((sizeof(char))*(strlen(del+1)));
   TEST_ERROR
   strcpy(str_right, (del+1)); //del+1 permette di non copiare il delimitatore
   TEST_ERROR
   
   
}

int reader_opt_conf(char* path, int* sim_time, int* prob_2, int* prob_3, int* prob_4, int* nof_invites, int* max_reject) {
   FILE* fd = fopen(path, "r");
   char buf[BUF_SIZE];
   
   while(fgets(buf, BUF_SIZE, fd)!=NULL) {
      if(strcmp(buf, "\n") || strcmp(buf, "")) { //se buf!='\n'
         char* attribute;
         char* value;
         int val;
         if(buf[strlen(buf)-1]=='\n') //elimina il \n a fine riga se c'è
            buf[strlen(buf)-1]='\0';
         divide(buf, attribute, value, '-'); //divide attributo e valore
         val=atoi(value);
         if (!strcmp(attribute, "sim_time")) 
               *sim_time = val;
         else if (!strcmp(attribute, "prob_2"))
               *prob_2 = val;
         else if (!strcmp(attribute, "prob_3"))
               *prob_3 = val;
         else if (!strcmp(attribute, "prob_4"))
               *prob_4 = val;
         else if (!strcmp(attribute, "nof_invites"))
               *nof_invites = val;
         else if (!strcmp(attribute, "max_reject"))
               *max_reject = val;
         else {
             printf("Errore nel file di configurazione\n");
             exit(EXIT_FAILURE);
          }
         
      }
   }
   fclose(fd);
   return 0;
}

