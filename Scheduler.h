#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Task.h"
#include <vector>
#include <stack>

using namespace std;

// Function declarations
void perform_primary_assignment(vector<Task>& allTasks, int totalCloudTaskTime);
void populate_weight_values(vector<Task>& allTasks, int totalCloudTaskTime);
void populate_priority_values(vector<Task>& allTasks);

Sequence execution_unit_selection(vector<Task>& allTasks, vector<int> priority_order, CloudTask cloudTaskTimes);
float compute_energy(vector<Task>& allTasks, CloudTask cloudTask, CoreTask coreTask);
float compute_time(vector<Task>& allTasks);
bool find_in_stack(stack<int> input, int val);
void kernel_function(vector<Task>& allTasksCopy, Sequence sequenceCopy, CloudTask cloudTask);
Sequence create_new_sequence(vector<Task>& allTasks, int target_task_id, int target_node, Sequence sequence);
void print_tasks(vector<Task>& allTasks);

// Sorting structures
struct SortByPriority {
    inline bool operator()(Task const& A, Task const& B) {
        return A.priority_value > B.priority_value;
    }
};

struct SortByTaskId {
    inline bool operator()(Task const& A, Task const& B) {
        return A.task_id < B.task_id;
    }
};

#endif // SCHEDULER_H
