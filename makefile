gestore: gestore.c sem_util.o conf_reader.o sig_util.o student header/shm_util.h header/error.h header/config.h header/stud.h
	@gcc gestore.c sem_util.o conf_reader.o sig_util.o -o gestore
	@rm -f *.o
	@echo Compilazione completata. Comandi:
	@echo "\e[31m./gestore\e[0m": esegui la simulazione
	@echo "\e[31mmake clean\e[0m": elimina i file eseguibili gestore e studente
student: student.c sem_util.o sig_util.o header/sem_util.h header/shm_util.h header/error.h header/config.h header/stud.h
	@gcc student.c sem_util.o sig_util.o -o student
sem_util.o: header/sem_util.h module/sem_util.c header/error.h header/config.h
	@gcc -c module/sem_util.c
sig_util.o: header/sig_util.h module/sig_util.c header/error.h header/config.h
	@gcc -c module/sig_util.c
conf_reader.o: header/conf_reader.h module/conf_reader.c header/error.h header/config.h
	@gcc -c module/conf_reader.c
clean:
	@rm gestore student
