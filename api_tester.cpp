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

slurm_job_info_t *  getJobInfo(job_info_msg_t * job_info_msg_ptr, unsigned jobid) {
        int result = slurm_load_job(&job_info_msg_ptr, jobid, SHOW_DETAIL);
        if (result == -1) {// There's some error
            if (job_info_msg_ptr != NULL)
                slurm_free_job_info_msg(job_info_msg_ptr);
            return NULL;
        }
        return job_info_msg_ptr->job_array[0];
}

unsigned suspendJob(slurm_job_info_t job) {
    job_info_msg_t * job_info_msg_ptr;
    unsigned jobid = job.job_id;
    unsigned state;
    int result = slurm_suspend(jobid);
    if (result == 0) {
        slurm_job_info_t * newjob;
        newjob = getJobInfo(job_info_msg_ptr, jobid);
        if (newjob == NULL) {// Can't find this job
            printf("Job %u can't be found after it's been suspended!\n", jobid);
            slurm_free_job_info_msg(job_info_msg_ptr);
            return job_states(JOB_END);
        }
        printf("Job %u has been suspended: ", jobid);
        outputJob(*newjob);
        state = newjob->job_state;
    }
    else {
        printf("Job %u hasn't been suspended successfully! The error code is: %d\n", jobid, result);
        slurm_free_job_info_msg(job_info_msg_ptr);
        return result;
    }
    slurm_free_job_info_msg(job_info_msg_ptr);
    return state;
}

unsigned resumeJob(slurm_job_info_t job) {
    unsigned jobid = job.job_id;
    slurm_job_info_t * newjob;
    int result = slurm_resume(jobid);
    if (result == 0) {
        newjob = getJobInfo(jobid);
        if (newjob == NULL) {// Can't find this job
            printf("Job %u can't be found after it's been resumed!\n", jobid);
            return job_states(JOB_END);
        }
        printf("Job %u has been resumed: ", jobid);
        outputJob(*newjob);
    }
    else
        printf("Job %u hasn't been resumed successfully! The error code is: %d\n", jobid, result);
    return newjob->job_state;
}

int submitJob(char* job_name, char* script) {
    job_desc_msg_t desc;
    submit_response_msg_t *resp;
    printf("The initial setting of desc: max_nodes = %d\n", desc.max_nodes);
    printf("The initial setting of desc: cluster = %s\n", desc.clusters);
    slurm_init_job_desc_msg(&desc);
    printf("The initial setting of desc: max_nodes = %d\n", desc.max_nodes);
    printf("The initial setting of desc: cluster = %s\n", desc.clusters);
    desc.max_nodes = 2;
    desc.name = job_name;
    desc.immediate = 0;
    desc.wait_all_nodes = 0;
    desc.script = script;
    int result = slurm_job_will_run(&desc);
    if (result == SLURM_SUCCESS)
        printf("The job will be run!\n");
    else
        printf("The job won't be run!\n");
    
    result = slurm_submit_batch_job(&desc, &resp);
    if (result == SLURM_SUCCESS)
        printf("The job runs!\n");
    else {
        printf("The job failed to be submitted with errno: %d!\n", result);
        printf("error_code = %p\n", &resp);
    }
    return result;
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
        char job_name[] = "Job cpi7 sent using C API";
        char script[] = "/home/opc/cloud/cpi_batch.sh";
        submitJob(job_name, script);
        outputJobs(job_info_msg_ptr);
        slurm_free_job_info_msg(job_info_msg_ptr);
}
