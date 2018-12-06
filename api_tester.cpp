/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <slurm/slurm.h>

int main(int argc, char *argv[])
{
        printf("Slurm version is: %ld\n", SLURM_VERSION_NUMBER);
        job_info_msg_t *job_info_msg_ptr = NULL;
        slurm_load_jobs((*time_t)NULL, &job_info_msg_ptr, SHOW_DETAIL);
        slurm_print_job_info_msg(stdout, job_info_msg_ptr, 0);
}
