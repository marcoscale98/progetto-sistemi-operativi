gestore: gestore.c sem_util.o conf_reader.o sig_util.o student header/shm_util.h header/error.h header/config.h header/stud.h opt_generator
	@gcc gestore.c sem_util.o conf_reader.o sig_util.o -lrt -o gestore
	@rm -f *.o
	@echo Compilazione completata:
	@echo "\e[31m./opt_generator\e[0m": genera dei parametri casuali per opt.conf #poi lo togliamo
	@echo "\e[31m./gestore\e[0m": esegui la simulazione
	@echo "\e[31mCtrl+C\e[0m": interrompi la simulazione
	@echo "\e[31mmake log\e[0m": esamina il file di log	#poi lo togliamo
	@echo "\e[31mmake clean\e[0m": elimina i file eseguibili
student: student.c sem_util.o sig_util.o header/sem_util.h header/shm_util.h header/error.h header/config.h header/stud.h
	@gcc student.c sem_util.o sig_util.o -o student
opt_generator: opt_generator.c header/conf_reader.h header/error.h
	@gcc opt_generator.c -o opt_generator
sem_util.o: header/sem_util.h module/sem_util.c header/error.h header/config.h
	@gcc -c module/sem_util.c
sig_util.o: header/sig_util.h module/sig_util.c header/error.h header/config.h
	@gcc -c module/sig_util.c
conf_reader.o: header/conf_reader.h module/conf_reader.c header/error.h header/config.h
	@gcc -c module/conf_reader.c
clean:
	@rm gestore student opt_generator
log:
	@cat logfile.log
