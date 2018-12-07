#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <slurm/slurm.h>

void outputJob(slurm_job_info_t job) {
    char * alloc_node = job.alloc_node;
    unsigned job_id = job.job_id;
    char * command = job.command;
    printf("Job ID: %d, allocated to node: %s and its state is: %d\n", job_id, alloc_node, job.job_state);
}

void outputJobs(job_info_msg_t *job_info_msg_ptr) {
    int job_cnt = job_info_msg_ptr->record_count;
    slurm_job_info_t *jobs = job_info_msg_ptr->job_array;
    for(int i=0; i<job_cnt ; i++) {
        outputJob(jobs[i]);
    }
}

slurm_job_info_t getJobInfo(unsigned jobid) {
        job_info_msg_t *job_info_msg_ptr = NULL;
        slurm_load_jobs((time_t)NULL, &job_info_msg_ptr, SHOW_DETAIL);
        int job_cnt = job_info_msg_ptr->record_count;
        slurm_job_info_t *jobs = job_info_msg_ptr->job_array;
        for(int i=0; i<job_cnt ; i++) {
            if (jobs[i].job_id == jobid)
                return jobs[i];
        }
        return jobs[0];
}

unsigned suspendJob(slurm_job_info_t job) {
    unsigned jobid = job.job_id;
    slurm_job_info_t newjob = getJobInfo(jobid);
    int result = slurm_suspend(job.job_id);
    if (result == 0) {
        while (newjob.job_state != job_states(JOB_SUSPENDED)) {
            sleep(1);
            newjob = getJobInfo(jobid);
        }
        printf("Job %u has been suspended: ", jobid);
        outputJob(newjob);
    }
    else
        printf("Job %u hasn't been suspended successfully! The error code is: %d\n", jobid, result);
    return newjob.job_state;
}

unsigned resumeJob(slurm_job_info_t job) {
    unsigned jobid = job.job_id;
    slurm_job_info_t newjob = getJobInfo(jobid);
    int result = slurm_resume(job.job_id);
    if (result == 0) {
        while (newjob.job_state != job_states(JOB_RUNNING) && newjob.job_state != job_states(JOB_PENDING)) {
            sleep(1);
            newjob = getJobInfo(jobid);
        }
        printf("Job %u has been resumed: ", jobid);
        outputJob(newjob);
    }
    else
        printf("Job %u hasn't been resumed successfully! The error code is: %d\n", jobid, result);
    return newjob.job_state;
}

int main(int argc, char *argv[])
{
        printf("Slurm version is: %ld\n", SLURM_VERSION_NUMBER);
        job_info_msg_t *job_info_msg_ptr = NULL;
        slurm_load_jobs((time_t)NULL, &job_info_msg_ptr, SHOW_DETAIL);
        slurm_print_job_info_msg(stdout, job_info_msg_ptr, 0);
        outputJobs(job_info_msg_ptr);
        int job_cnt = job_info_msg_ptr->record_count;
        slurm_job_info_t *jobs = job_info_msg_ptr->job_array;
        for(int i=0; i<job_cnt ; i++) {
            suspendJob(jobs[i]);
            resumeJob(jobs[i]);
        }
}
