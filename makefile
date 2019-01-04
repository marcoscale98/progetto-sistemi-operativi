gestore: gestore.c sem_util.o conf_reader.o time_util.o sig_util.o student header/msg_util.h header/shm_util.h header/error.h header/config.h
	@gcc gestore.c sem_util.o conf_reader.o time_util.o sig_util.o -o gestore
	@rm -f *.o
	@echo Compilazione completata.
	@echo ./gestore:	esegui la simulazione
	@echo make clean:	elimina i file eseguibili gestore e studente
	@echo make clean.o:	elimina i file oggetto *.o
student: student.c sem_util.o header/msg_util.h header/shm_util.h header/error.h header/config.h header/msg_util.h
	@gcc student.c sem_util.o -o student
sem_util.o: header/sem_util.h module/sem_util.c header/error.h header/config.h
	@gcc -c module/sem_util.c
sig_util.o: header/sig_util.h module/sig_util.c header/error.h header/config.h
	@gcc -c module/sig_util.c
conf_reader.o: header/conf_reader.h module/conf_reader.c header/error.h header/config.h
	@gcc -c module/conf_reader.c
time_util.o: header/time_util.h module/time_util.c header/error.h header/config.h
	@gcc -c module/time_util.c
clean:
	@rm gestore student
clean.o:
	@rm -f *.o
