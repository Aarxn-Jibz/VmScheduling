#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define STACK_SIZE 256
#define MAX_TASKS 100
#define QUANTUM 5
#define HUGE_PROG_SIZE 15000

extern void score(double *input, double *output);

typedef enum { LOAD_VAL, ADD, IO_WAIT, HALT } Opcode;
typedef enum { READY, RUNNING, HALTED } TaskState;

typedef struct {
  int task_id;
  TaskState state;
  int ip;
  int sp;
  int stack[STACK_SIZE];
  int *program;
  int cpu_ops;
  int io_wait_ops;
  long long turnaround_time;
  int priority; // NEW: For MLFQ to track which queue it belongs in
} TCB;

typedef struct {
  TCB *tasks[MAX_TASKS];
  int front;
  int rear;
  int count;
} Queue;

void init_queue(Queue *q) {
  q->front = 0;
  q->rear = -1;
  q->count = 0;
}
void enqueue(Queue *q, TCB *task) {
  if (q->count >= MAX_TASKS)
    return;
  q->rear = (q->rear + 1) % MAX_TASKS;
  q->tasks[q->rear] = task;
  q->count++;
}
void enqueue_front(Queue *q, TCB *task) {
  if (q->count >= MAX_TASKS)
    return;
  q->front = (q->front - 1 + MAX_TASKS) % MAX_TASKS;
  q->tasks[q->front] = task;
  q->count++;
}
TCB *dequeue(Queue *q) {
  if (q->count == 0)
    return NULL;
  TCB *task = q->tasks[q->front];
  q->front = (q->front + 1) % MAX_TASKS;
  q->count--;
  return task;
}

long long system_ticks = 0;

void execute_quantum(TCB *task) {
  task->state = RUNNING;
  int instructions_executed = 0;

  while (instructions_executed < QUANTUM && task->state != HALTED) {
    system_ticks++;
    int instruction = task->program[task->ip];

    switch (instruction) {
    case LOAD_VAL:
      task->ip++;
      task->stack[++task->sp] = task->program[task->ip];
      task->cpu_ops++;
      break;
    case ADD:
      if (task->sp >= 1) {
        int a = task->stack[task->sp--];
        int b = task->stack[task->sp--];
        task->stack[++task->sp] = a + b;
      }
      task->cpu_ops++;
      break;
    case IO_WAIT:
      task->io_wait_ops++;
      instructions_executed = QUANTUM; // Yield CPU
      break;
    case HALT:
      task->state = HALTED;
      task->turnaround_time = system_ticks;
      break;
    }
    task->ip++;
    instructions_executed++;
  }
  if (task->state != HALTED)
    task->state = READY;
}

// SCHEDULER 1 & 2: DUMB AND SMART
double run_scheduler(Queue *ready_queue, bool smart_mode) {
  system_ticks = 0;
  double total_turnaround = 0;
  int tasks_completed = 0;

  while (ready_queue->count > 0) {
    TCB *current_task = dequeue(ready_queue);
    execute_quantum(current_task);

    if (current_task->state != HALTED) {
      if (smart_mode) {
        int total_ops = current_task->cpu_ops + current_task->io_wait_ops;
        if (total_ops == 0)
          total_ops = 1;

        double projected_cpu =
            ((double)current_task->cpu_ops / total_ops) * 20.0;
        double projected_io =
            ((double)current_task->io_wait_ops / total_ops) * 20.0;

        double features[2] = {projected_cpu, projected_io};
        double output_scores[2];
        score(features, output_scores);

        double prediction = (output_scores[1] > output_scores[0]) ? 1.0 : 0.0;

        if (prediction == 1.0)
          enqueue_front(ready_queue, current_task);
        else
          enqueue(ready_queue, current_task);
      } else {
        enqueue(ready_queue, current_task);
      }
    } else {
      total_turnaround += current_task->turnaround_time;
      tasks_completed++;
    }
  }
  return total_turnaround / tasks_completed;
}

// SCHEDULER 3: MULTI-LEVEL FEEDBACK QUEUE (MLFQ)
double run_mlfq_scheduler(TCB *tasks_array, int num_tasks) {
  Queue mlfq[3]; // 3 Priority Levels: 0 (High), 1 (Medium), 2 (Low)
  init_queue(&mlfq[0]);
  init_queue(&mlfq[1]);
  init_queue(&mlfq[2]);

  // All tasks start in the highest priority queue
  for (int i = 0; i < num_tasks; i++)
    enqueue(&mlfq[0], &tasks_array[i]);

  system_ticks = 0;
  double total_turnaround = 0;
  int tasks_completed = 0;

  while (tasks_completed < num_tasks) {
    TCB *current_task = NULL;

    // Find a task in the highest available queue
    for (int i = 0; i < 3; i++) {
      if (mlfq[i].count > 0) {
        current_task = dequeue(&mlfq[i]);
        break;
      }
    }
    if (!current_task)
      break;

    int io_before = current_task->io_wait_ops;
    execute_quantum(current_task);
    int io_after = current_task->io_wait_ops;

    if (current_task->state != HALTED) {
      if (io_after > io_before) {
        // I/O Bound: Yielded early! Keep it at its current high priority.
        enqueue(&mlfq[current_task->priority], current_task);
      } else {
        // CPU Bound: Hogged the whole time slice! Punish it by demoting.
        if (current_task->priority < 2)
          current_task->priority++;
        enqueue(&mlfq[current_task->priority], current_task);
      }
    } else {
      total_turnaround += current_task->turnaround_time;
      tasks_completed++;
    }
  }
  return total_turnaround / tasks_completed;
}

void reset_task(TCB *task) {
  task->state = READY;
  task->ip = 0;
  task->sp = -1;
  task->cpu_ops = 0;
  task->io_wait_ops = 0;
  task->turnaround_time = 0;
  task->priority = 0; // Reset to highest priority queue
}

static int cpu_progs[20][HUGE_PROG_SIZE];
static int io_progs[50][150];
static TCB tasks[70];

int main() {
  printf("--- Generating Torture Test Workload ---\n");

  for (int i = 0; i < 20; i++) {
    int len = 0;
    cpu_progs[i][len++] = LOAD_VAL;
    cpu_progs[i][len++] = 1;
    for (int j = 0; j < 3000; j++) {
      cpu_progs[i][len++] = LOAD_VAL;
      cpu_progs[i][len++] = 1;
      cpu_progs[i][len++] = ADD;
    }
    cpu_progs[i][len++] = HALT;
    tasks[i].task_id = i + 1;
    tasks[i].program = cpu_progs[i];
    reset_task(&tasks[i]);
  }

  for (int i = 0; i < 50; i++) {
    int idx = i + 20;
    int len = 0;
    for (int j = 0; j < 100; j++)
      io_progs[i][len++] = IO_WAIT;
    io_progs[i][len++] = HALT;
    tasks[idx].task_id = idx + 1;
    tasks[idx].program = io_progs[i];
    reset_task(&tasks[idx]);
  }

  Queue queue;

  // RACE 1: DUMB (ROUND ROBIN)
  init_queue(&queue);
  for (int i = 0; i < 70; i++)
    enqueue(&queue, &tasks[i]);
  clock_t start_dumb = clock();
  double dumb_avg = run_scheduler(&queue, false);
  double time_dumb = (double)(clock() - start_dumb) / CLOCKS_PER_SEC;

  // RACE 2: MLFQ (INDUSTRY STANDARD)
  for (int i = 0; i < 70; i++)
    reset_task(&tasks[i]);
  clock_t start_mlfq = clock();
  double mlfq_avg = run_mlfq_scheduler(tasks, 70);
  double time_mlfq = (double)(clock() - start_mlfq) / CLOCKS_PER_SEC;

  // RACE 3: SMART (AI-AUGMENTED)
  for (int i = 0; i < 70; i++)
    reset_task(&tasks[i]);
  init_queue(&queue);
  for (int i = 0; i < 70; i++)
    enqueue(&queue, &tasks[i]);
  clock_t start_smart = clock();
  double smart_avg = run_scheduler(&queue, true);
  double time_smart = (double)(clock() - start_smart) / CLOCKS_PER_SEC;

  // THE RESULTS
  printf("\n====================================================\n");
  printf("              BENCHMARK RESULTS                     \n");
  printf("====================================================\n");
  printf("1. Standard Round Robin (Dumb Scheduler):\n");
  printf("   Avg Turnaround Time:     %.0f CPU cycles\n\n", dumb_avg);

  printf("2. Multi-Level Feedback Queue (Industry Standard):\n");
  printf("   Avg Turnaround Time:     %.0f CPU cycles\n\n", mlfq_avg);

  printf("3. ML-Augmented (Smart Scheduler):\n");
  printf("   Avg Turnaround Time:     %.0f CPU cycles\n\n", smart_avg);

  double dumb_to_smart = ((dumb_avg - smart_avg) / dumb_avg) * 100.0;
  double mlfq_to_smart = ((mlfq_avg - smart_avg) / mlfq_avg) * 100.0;

  printf("=> AI vs DUMB:  Improved efficiency by %.2f%%\n", dumb_to_smart);
  printf("=> AI vs MLFQ:  Improved efficiency by %.2f%%\n", mlfq_to_smart);
  printf("====================================================\n");

  return 0;
}
