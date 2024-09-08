---

# **Paper Implementation: Energy and Performance-Aware Task Scheduling in a Mobile Cloud Computing Environment**

## **Overview**

This script implements the task scheduling algorithm described in the paper **["Energy and Performance-Aware Task Scheduling in a Mobile Cloud Computing Environment"](https://web.archive.org/web/20140904171728id_/http://sportlab.usc.edu/~massoud/Papers/task-sched-MCC-cloud14.pdf)** by Xue Lin, Yanzhi Wang, Qing Xie, and Massoud Pedram from the Department of Electrical Engineering at the University of Southern California.

## **Description**

The algorithm efficiently schedules tasks in a mobile cloud computing environment by balancing the dual objectives of **minimizing energy consumption** and **execution time**. It models both local and cloud processing capabilities and times, as well as the energy costs associated with executing tasks in these environments.

## **Implementation Details**

- The script defines a `Task` class representing a task with attributes such as task ID, parent and child tasks, core and cloud processing speeds, and scheduling times.
- The main algorithm initializes the tasks, sets their priorities, and iteratively schedules them on either local cores or the cloud based on a series of calculated metrics, optimizing the balance between energy efficiency and performance.

---

## **Project Structure**

```
/scheduler_project
    ├── Task.h             # Class definitions for Task, CloudTask, CoreTask, Sequence
    ├── Task.cpp           # Task-related implementations
    ├── Scheduler.h        # Function declarations for scheduling algorithms
    ├── Scheduler.cpp      # Task scheduling logic and optimization functions
    ├── main.cpp           # Main driver script
    └── README.md          # Project README
```

---

## **Features**
- **Dynamic Task Assignment**: Schedules tasks to local cores or cloud based on priority and execution cost.
- **Energy and Time Optimization**: Iteratively optimizes task distribution for minimal energy consumption and execution time.
- **Task Dependencies**: Manages task dependencies to ensure proper execution order.
- **Iterative Refinement**: Refines task assignments over multiple iterations for better results.

---

## **Key Observations**

- **Initial Scheduling Algorithm**: Initially, tasks are mostly assigned to **Core 3**, as the algorithm tries to minimize execution time. However, this can lead to higher energy consumption.
  
- **Kernel Algorithm**: The kernel optimization phase typically assigns tasks to **Core 1** and the **cloud**. It strategically schedules tasks to **Core 1** to minimize energy usage while leveraging cloud resources to handle time constraints, reducing overall energy consumption.
  
- **Time Constraint Impact**: Without time constraints, the algorithm tends to schedule all tasks to the **cloud**, as it becomes the most energy-efficient option, despite potentially longer execution times.


---
