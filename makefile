gestore: gestore.c sem_util.o conf_reader.o time_util.o sig_util.o student error.h
	@gcc gestore.c sem_util.o conf_reader.o time_util.o sig_util.o -o gestore
	@rm -f *.o
	@echo Compilazione completata. Eseguire con ./gestore
student: student.c shm_util.h error.h config.h sig_util.h sem_util.o
	@gcc student.c sem_util.o -o student
sem_util.o: header/sem_util.h module/sem_util.c error.h
	@gcc -c module/sem_util.c
sig_util.o: header/sig_util.h module/sig_util.c error.h
	@gcc -c module/sig_util.c
conf_reader.o: header/conf_reader.h module/conf_reader.c error.h
	@gcc -c module/conf_reader.c
time_util.o: header/time_util.h module/time_util.c error.h
	@gcc -c module/time_util.c
	
