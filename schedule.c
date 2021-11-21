#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct Job
{
  int id;
  int arrival;
  int burst;
  int sjf_done;
  int sjrf_done;
  int rr_done;
};

void swap(int *a, int *b)
{
  int temp = *a;
  *a = *b;
  *b = temp;
}

int find_fcfs_tat(struct Job *jobs, int size, int verbose)
{
  int wt[size], tat[size], total_tat = 0, i, j;
  float avg_tat;

  // calculate waiting time
  wt[0] = 0;
  for (i = 1; i < size; i++)
  {
    // wt[i] = jobs[i - 1].burst + wt[i - 1] - jobs[i].arrival;
    wt[i] = 0;
    for (j = 0; j < i; j++)
    {
      wt[i] += jobs[j].burst;
    }
    wt[i] -= jobs[i].arrival;
  }

  // calculate turn around time for each process & total turnaround time
  for (i = 0; i < size; i++)
  {
    // tat[i] = jobs[i].burst + wt[i];
    tat[i] = 0;
    for (j = 0; j <= i; j++)
    {
      tat[i] += jobs[j].burst;
    }
    tat[i] -= jobs[i].arrival;
    total_tat += tat[i];
    if (verbose)
    {
      printf("id: %d\t\tarrival: %d\t\tburst: %d\t\tfcfs_tat: %d\t\tfcfs_wt: %d\n",
             i, jobs[i].arrival, jobs[i].burst, tat[i], wt[i]);
    }
  }

  // calculate avg turnaround time
  avg_tat = total_tat / size;

  return round(avg_tat);
}

int find_sjf_tat(struct Job *jobs, int size, int verbose)
{
  int wt[size], tat[size], total_tat = 0, i, time = 0, count = 0;
  float avg_tat;
  struct Job shortest;

  // for all jobs not completed
  while (count < size)
  {
    // get the first available job
    for (i = 0; i < size; i++)
    {
      if (jobs[i].arrival <= time && jobs[i].sjf_done == 0)
      {
        shortest = jobs[i];
        break;
      }
    }

    // find the shortest job among jobs that have arrived
    for (i = 0; i < size; i++)
    {
      // among jobs available find the shortest one
      if (jobs[i].arrival <= time && jobs[i].sjf_done == 0 && shortest.burst > jobs[i].burst)
      {
        shortest = jobs[i];
      }
    }

    // calculate wt and tat for shortest job
    wt[shortest.id - 1] = time - shortest.arrival;
    tat[shortest.id - 1] = time - shortest.arrival + shortest.burst;

    // mark the shortest as done for sjf
    jobs[shortest.id - 1].sjf_done = 1;

    // update helper variables
    total_tat += tat[shortest.id - 1];
    time += shortest.burst;
    count++;

    // display job completion details
    if (verbose)
    {
      printf("id:%d completed at: %d\t\ttat:%d\t\twt:%d\n",
             shortest.id, time, tat[shortest.id - 1], wt[shortest.id - 1]);
    }
  }

  avg_tat = total_tat / size;

  return round(avg_tat);
}
int find_sjrf_tat(struct Job *jobs, int size, int verbose)
{
  int wt[size], tat[size], bt[size], total_tat = 0, i, time = 0, count = 0;
  float avg_tat;
  struct Job shortest;

  // init bt to track how much time the progress needs to finish
  for (i = 0; i < size; i++)
  {
    bt[i] = jobs[i].burst;
  }

  // for all jobs not completed
  while (count < size)
  {
    // get the first arrived and incomplete job
    for (i = 0; i < size; i++)
    {
      if (jobs[i].arrival <= time && jobs[i].sjrf_done == 0)
      {
        shortest = jobs[i];
        break;
      }
    }

    // find the shortest job among jobs that have arrived
    for (i = 0; i < size; i++)
    {
      // among jobs arrived find the shortest incomplete one
      if (jobs[i].arrival <= time && jobs[i].sjrf_done == 0 && bt[shortest.id - 1] > bt[i])
      {
        shortest = jobs[i];
      }
    }

    // update time
    time++;

    // subtract 1 time unit from shortest jobs bt
    bt[shortest.id - 1]--;

    // if the job is completed
    if (bt[shortest.id - 1] <= 0)
    {
      // set it as completed & calculate turnaround time
      jobs[shortest.id - 1].sjrf_done = 1;
      tat[shortest.id - 1] = time - jobs[shortest.id - 1].arrival;
      wt[shortest.id - 1] = time - jobs[shortest.id - 1].arrival - jobs[shortest.id - 1].burst;
      total_tat += tat[shortest.id - 1];
      // update completed job count
      count++;

      // display job completion details
      if (verbose)
      {
        printf("id:%d completed at: %d\t\ttat:%d\t\twt:%d\n",
               shortest.id, time, tat[shortest.id - 1], wt[shortest.id - 1]);
      }
    }
  }

  avg_tat = total_tat / size;

  return round(avg_tat);
}

int find_rr_tat(struct Job *jobs, int size, int time_quantum, int verbose)
{
  int tat[size], bt[size], job_queue[size];
  int total_tat = 0, time = 0, count = 0, queued = 0, i, j, job_finished = 0, burst_count = 0;
  float avg_tat;

  // init job queue to denote that no job has arrived yet
  // -1 => job not arrived
  // -2 => job complete
  // 0+ => order in queue, 0 being next job in queue
  for (i = 0; i < size; i++)
  {
    job_queue[i] = -1;
  }

  // assume the first job in input's arrival is 0
  job_queue[0] = 0;
  queued = 1;

  // init bt to track how much time the progress needs to finish
  for (i = 0; i < size; i++)
  {
    bt[i] = jobs[i].burst;
  }

  // for all jobs not complete
  i = 0;
  burst_count++;
  while (count < size)
  {
    // apply time_quantum burst
    // job will be finished this time_quantum
    if (bt[i] <= time_quantum && bt[i] > 0)
    {
      time += bt[i];
      bt[i] = 0;
      job_finished = 1;
    }
    // remaining time of job is longer than time quantum
    else if (bt[i] > 0)
    {
      // update remaining burst
      bt[i] -= time_quantum;
      time += time_quantum;
    }

    // for all jobs
    for (j = 0; j < size; j++)
    {
      // if a job arrived add it to the job queue
      if (job_queue[j] == -1 && jobs[j].arrival <= time)
      {
        job_queue[j] = queued;
        queued++;
      }
    }
    // if the job is finished update queue status, queued job count, completed ojb count and tat
    if (job_finished == 1)
    {
      job_queue[i] = -2;
      queued--;
      count++;
      tat[i] = time - jobs[i].arrival;
      total_tat += tat[i];
      job_finished = 0;

      if (verbose)
      {
        printf("id:%d \t\tcompleted at: %d\t\ttat:%d\n",
               jobs[i].id, time, tat[i]);
      }
    }
    // update job queue order values
    for (j = 0; j < size; j++)
    {
      // shift items to higher priority
      if (job_queue[j] > 0)
      {
        job_queue[j]--;
      }
      // put the executed job to the end of the list
      else if (job_queue[j] == 0)
      {
        job_queue[j] = queued - 1;
      }

      // select the index of job with queue order 0 for next execution
      if (job_queue[j] == 0)
      {
        i = j;
        if (verbose)
        {
          printf("Burst: %d\t\tSelected: %d\t\tRemaing: %d\n", burst_count, jobs[i].id, bt[i]);
        }
        burst_count++;
      }
    }
  }

  avg_tat = total_tat / size;

  return round(avg_tat);
}

int main(int argc, char *argv[])
{
  int time_quantum;
  FILE *fp;
  struct Job *jobs;
  int count = 0;
  char chr;
  char line[128];
  int fcfs_avg_tat, sjf_avg_tat, sjrf_avg_tat, rr_avg_tat;

  // Check arguments
  if (argc != 3)
  {
    printf("[ERROR] - Incorrect amount of arguments supplied!");
    exit(EXIT_FAILURE);
  }

  // Assign and check time quantum
  time_quantum = atoi(argv[2]);

  if (time_quantum > 400 || time_quantum < 5)
  {
    printf("[ERROR] - Time quantum not in range [5- 400]!");
    exit(EXIT_FAILURE);
  }

  // Open File
  fp = fopen(argv[1], "r");
  if (fp == NULL)
  {
    printf("[ERROR] - Could not open file");
    exit(EXIT_FAILURE);
  }

  // Get number of lines (jobs)
  for (chr = getc(fp); chr != EOF; chr = getc(fp))
  {
    if (chr == '\n')
    {
      count = count + 1;
    }
  }

  count++;

  // allocate space for jobs
  jobs = malloc(count * sizeof(struct Job));

  // get job data from file line by line
  rewind(fp); // reset fp to start of file
  count = 0;

  // read and assign job data from file
  while (fgets(line, sizeof(line), fp) != NULL)
  {
    jobs[count].id = atoi(strtok(line, " "));
    jobs[count].arrival = atoi(strtok(NULL, " "));
    jobs[count].burst = atoi(strtok(NULL, " "));
    jobs[count].sjf_done = 0;
    jobs[count].sjrf_done = 0;
    jobs[count].rr_done = 0;

    count++;
  }

  fcfs_avg_tat = find_fcfs_tat(jobs, count, 0);
  sjf_avg_tat = find_sjf_tat(jobs, count, 0);
  sjrf_avg_tat = find_sjrf_tat(jobs, count, 0);
  rr_avg_tat = find_rr_tat(jobs, count, time_quantum, 0);

  printf("FCFS:\t%d", fcfs_avg_tat);
  printf("\nSJF:\t%d", sjf_avg_tat);
  printf("\nSJRF:\t%d", sjrf_avg_tat);
  printf("\nRR:\t%d", rr_avg_tat);

  exit(EXIT_SUCCESS);
}