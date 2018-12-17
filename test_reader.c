#include "reader_opt_conf.c"

int main() {
   char* path = "opt.conf";
   int sim_time=0, prob_2=0, prob_3=0, prob_4=0, nof_invites=0, max_reject=0;
   
   reader_opt_conf(path, &sim_time, &prob_2, &prob_3, &prob_4, &nof_invites, &max_reject);
   printf("prova\n");
   
   printf("sim_time = %d\n", sim_time);
   printf("prob_2 = %d\n", prob_2);
   printf("prob_3 = %d\n", prob_3);
   printf("prob_4 = %d\n", prob_4);
   printf("nof_invites = %d\n", nof_invites);
   printf("max_reject = %d\n", max_reject);
   return 0;
}
