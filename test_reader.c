#include "reader_opt_conf.c"

int main() {
   char* path = "opt.conf";
   int sim_time, prob_2, prob_3, prob_4, nof_invites, max_reject;
   
   reader_opt_conf(path, &sim_time, &prob_2, &prob_3, &prob_4, &nof_invites, &max_reject);

   printf("sim_time = %d\n", sim_time);
   printf("sim_time = %d\n", prob_2);
   printf("sim_time = %d\n", prob_3);
   printf("sim_time = %d\n", prob_4);
   printf("sim_time = %d\n", nof_invites);
   printf("sim_time = %d\n", max_reject);
   return 0;
}
