#include "Scheduler.h"
#include <iostream>
#include <algorithm>

using namespace std;

// Function to assign primary tasks to core or cloud
void perform_primary_assignment(vector<Task>& allTasks, int totalCloudTaskTime) {
    allTasks[0].is_core = true;
    allTasks[allTasks.size() - 1].is_core = true;

    for (int i = 1; i < allTasks.size() - 1; i++) {
        allTasks[i].is_core = false;
        for (int j = 0; j < 3; j++) {
            if (allTasks[i].core_speed[j] < totalCloudTaskTime) {
                allTasks[i].is_core = true;
            }
        }
    }
}

// Function to populate initial weight values
void populate_weight_values(vector<Task>& allTasks, int totalCloudTaskTime) {
    for (int i = 0; i < allTasks.size(); i++) {
        if (allTasks[i].is_core) {
            allTasks[i].weight_value = (float)(allTasks[i].core_speed[0] + allTasks[i].core_speed[1] + allTasks[i].core_speed[2]) / 3;
        } else {
            allTasks[i].weight_value = totalCloudTaskTime;
        }
    }
}

// Function to populate priority values
void populate_priority_values(vector<Task>& allTasks) {
    int total_tasks = allTasks.size();

    for (int i = total_tasks - 1; i >= 0; i--) {
        float task_weight = allTasks[i].weight_value;

        if (allTasks[i].child_task_ids.size() == 0) {
            allTasks[i].priority_value = task_weight;
            continue;
        }

        float child_weight_score = 0;
        for (int task_id : allTasks[i].child_task_ids) {
            if (allTasks[task_id - 1].priority_value > child_weight_score)
                child_weight_score = allTasks[task_id - 1].priority_value;
        }

        allTasks[i].priority_value = child_weight_score + task_weight;
    }
}

// Function for execution unit selection
Sequence execution_unit_selection(vector<Task>& allTasks, vector<int> priority_order, CloudTask cloudTaskTimes) {
    int local_status[3] = {0, 0, 0};
    int cloud_status[3] = {0, 0, 0};

    Sequence sequence;

    for (int current_task_id : priority_order) {
        if (allTasks[current_task_id - 1].is_core == true) {
            // Updating localCompute ready time
            if (allTasks[current_task_id - 1].parent_task_ids.size() == 0) {
                allTasks[current_task_id - 1].localCompute.ready_time = 0;
            } else {
                for (int task_id : allTasks[current_task_id - 1].parent_task_ids) {
                    int prev_finish_time = max(allTasks[task_id - 1].localCompute.finish_time, allTasks[task_id - 1].wirelessReceive.finish_time);
                    if (prev_finish_time > allTasks[current_task_id - 1].localCompute.ready_time)
                        allTasks[current_task_id - 1].localCompute.ready_time = prev_finish_time;
                }
            }

            int core_1_finishtime = max(local_status[0], allTasks[current_task_id - 1].localCompute.ready_time) + allTasks[current_task_id - 1].core_speed[0];
            int core_2_finishtime = max(local_status[1], allTasks[current_task_id - 1].localCompute.ready_time) + allTasks[current_task_id - 1].core_speed[1];
            int core_3_finishtime = max(local_status[2], allTasks[current_task_id - 1].localCompute.ready_time) + allTasks[current_task_id - 1].core_speed[2];

            int core_allocated = 0;
            int core_assign_finishtime = core_1_finishtime;

            if (core_assign_finishtime > core_2_finishtime) {
                core_assign_finishtime = core_2_finishtime;
                core_allocated = 1;
            }
            if (core_assign_finishtime > core_3_finishtime) {
                core_assign_finishtime = core_3_finishtime;
                core_allocated = 2;
            }

            // Updating finish time, core assigned, and start time
            allTasks[current_task_id - 1].core_assigned = core_allocated;
            allTasks[current_task_id - 1].localCompute.finish_time = core_assign_finishtime;
            allTasks[current_task_id - 1].localCompute.start_time[core_allocated] = max(local_status[core_allocated], allTasks[current_task_id - 1].localCompute.ready_time);
            local_status[core_allocated] = core_assign_finishtime;

            // Adding task to sequence
            if (core_allocated == 0)
                sequence.core_1.push_back(current_task_id);
            else if (core_allocated == 1)
                sequence.core_2.push_back(current_task_id);
            else if (core_allocated == 2)
                sequence.core_3.push_back(current_task_id);

        } else {
            // Wireless send updates
            for (int task_id : allTasks[current_task_id - 1].parent_task_ids) {
                int wireless_send_time = max(allTasks[task_id - 1].localCompute.finish_time, allTasks[task_id - 1].wirelessSend.finish_time);
                if (wireless_send_time > allTasks[current_task_id - 1].wirelessSend.ready_time)
                    allTasks[current_task_id - 1].wirelessSend.ready_time = wireless_send_time;
            }

            // Wireless send stage
            allTasks[current_task_id - 1].wirelessSend.start_time = max(cloud_status[0], allTasks[current_task_id - 1].wirelessSend.ready_time);
            allTasks[current_task_id - 1].wirelessSend.finish_time = allTasks[current_task_id - 1].wirelessSend.start_time + cloudTaskTimes.wireless_send_time;
            cloud_status[0] = allTasks[current_task_id - 1].wirelessSend.finish_time;

            // Cloud compute updates
            int parent_max_cloud_compute_time = 0;
            for (int task_id : allTasks[current_task_id - 1].parent_task_ids) {
                if (allTasks[task_id - 1].cloudCompute.finish_time > parent_max_cloud_compute_time)
                    parent_max_cloud_compute_time = allTasks[task_id - 1].cloudCompute.finish_time;
            }

            // Cloud compute stage
            allTasks[current_task_id - 1].cloudCompute.ready_time = max(allTasks[current_task_id - 1].wirelessSend.finish_time, parent_max_cloud_compute_time);
            allTasks[current_task_id - 1].cloudCompute.start_time = max(cloud_status[1], allTasks[current_task_id - 1].cloudCompute.ready_time);
            int cloud_compute_finish_time = allTasks[current_task_id - 1].cloudCompute.start_time + cloudTaskTimes.compute_time;
            allTasks[current_task_id - 1].cloudCompute.finish_time = cloud_compute_finish_time;
            cloud_status[1] = cloud_compute_finish_time;

            // Wireless receive stage
            allTasks[current_task_id - 1].wirelessReceive.ready_time = allTasks[current_task_id - 1].cloudCompute.finish_time;
            allTasks[current_task_id - 1].wirelessReceive.start_time = max(cloud_status[2], allTasks[current_task_id - 1].wirelessReceive.ready_time);
            int cloud_wireless_receive_finish_time = allTasks[current_task_id - 1].wirelessReceive.start_time + cloudTaskTimes.wireless_receive_time;
            allTasks[current_task_id - 1].wirelessReceive.finish_time = cloud_wireless_receive_finish_time;
            cloud_status[2] = cloud_wireless_receive_finish_time;

            allTasks[current_task_id - 1].core_assigned = 3;
            sequence.cloud.push_back(current_task_id);
        }
    }
    return sequence;
}

// Function to compute energy consumption
float compute_energy(vector<Task>& allTasks, CloudTask cloudTask, CoreTask coreTask) {
    float total_energy = 0;

    for (int i = 0; i < allTasks.size(); i++) {
        if (allTasks[i].is_core == true) {
            int node = allTasks[i].core_assigned;
            total_energy += allTasks[i].core_speed[node] * coreTask.compute_costs[node];
        }
        if (allTasks[i].is_core == false) {
            total_energy += cloudTask.wireless_send_time * cloudTask.compute_costs[0];
        }
    }
    return total_energy;
}

// Function to compute total execution time
float compute_time(vector<Task>& allTasks) {
    float total_time = 0;

    for (int i = 0; i < allTasks.size(); i++) {
        if (allTasks[i].child_task_ids.size() == 0) {
            if (allTasks[i].is_core && allTasks[i].localCompute.finish_time > total_time)
                total_time = allTasks[i].localCompute.finish_time;

            if (!allTasks[i].is_core && allTasks[i].wirelessReceive.finish_time > total_time)
                total_time = allTasks[i].wirelessReceive.finish_time;
        }
    }
    return total_time;
}

// Function to find a value in a stack
bool find_in_stack(stack<int> input, int val) {
    while (!input.empty() && input.top() != val)
        input.pop();
    return !input.empty();
}

// Kernel function for task scheduling
void kernel_function(vector<Task>& allTasksCopy, Sequence sequenceCopy, CloudTask cloudTask) {
    int local_status[3] = {0, 0, 0};
    int cloud_status[3] = {0, 0, 0};

    int no_of_tasks = allTasksCopy.size();
    vector<int> ready1(no_of_tasks, -1);
    vector<int> ready2(no_of_tasks, -1);

    if (sequenceCopy.core_1.size() > 0)
        ready2[sequenceCopy.core_1[0] - 1] = 0;

    if (sequenceCopy.core_2.size() > 0)
        ready2[sequenceCopy.core_2[0] - 1] = 0;

    if (sequenceCopy.core_3.size() > 0)
        ready2[sequenceCopy.core_3[0] - 1] = 0;

    if (sequenceCopy.cloud.size() > 0)
        ready2[sequenceCopy.cloud[0] - 1] = 0;

    stack<int> toExecute;

    for (int i = 0; i < no_of_tasks; i++) {
        allTasksCopy[i].is_scheduled = false;

        if (allTasksCopy[i].parent_task_ids.size() == 0) {
            ready1[i] = 0;
            toExecute.push(allTasksCopy[i].task_id);
        }
    }

    while (!toExecute.empty()) {
        int current_task_id = toExecute.top();
        toExecute.pop();

        allTasksCopy[current_task_id - 1].is_scheduled = true;

        if (allTasksCopy[current_task_id - 1].is_core == true) {
            // Populating the ready time
            if (allTasksCopy[current_task_id - 1].parent_task_ids.size() == 0)
                allTasksCopy[current_task_id - 1].localCompute.ready_time = 0;
            else {
                for (int task_id : allTasksCopy[current_task_id - 1].parent_task_ids) {
                    int parent_task_finish_time = max(allTasksCopy[task_id - 1].localCompute.finish_time, allTasksCopy[task_id - 1].wirelessReceive.finish_time);
                    if (allTasksCopy[current_task_id - 1].localCompute.ready_time < parent_task_finish_time)
                        allTasksCopy[current_task_id - 1].localCompute.ready_time = parent_task_finish_time;
                }
            }
        }

        int core_assigned = allTasksCopy[current_task_id - 1].core_assigned;

        if (allTasksCopy[current_task_id - 1].is_core == true) {
            // Local compute stage
            allTasksCopy[current_task_id - 1].localCompute.start_time[core_assigned] = max(local_status[core_assigned], allTasksCopy[current_task_id - 1].localCompute.ready_time);
            allTasksCopy[current_task_id - 1].localCompute.finish_time = allTasksCopy[current_task_id - 1].localCompute.start_time[core_assigned] + allTasksCopy[current_task_id - 1].core_speed[core_assigned];
            allTasksCopy[current_task_id - 1].wirelessSend.start_time = -1;
            allTasksCopy[current_task_id - 1].cloudCompute.finish_time = -1;
            allTasksCopy[current_task_id - 1].wirelessReceive.finish_time = -1;
            local_status[core_assigned] = allTasksCopy[current_task_id - 1].localCompute.finish_time;
        }

        if (allTasksCopy[current_task_id - 1].is_core == false) {
            // Wireless send stage
            if (allTasksCopy[current_task_id - 1].parent_task_ids.size() == 0)
                allTasksCopy[current_task_id - 1].wirelessSend.ready_time = 0;
            else {
                for (int task_id : allTasksCopy[current_task_id - 1].parent_task_ids) {
                    int parents_wireless_send_time = max(allTasksCopy[task_id - 1].localCompute.finish_time, allTasksCopy[task_id - 1].wirelessSend.finish_time);
                    if (parents_wireless_send_time > allTasksCopy[current_task_id - 1].wirelessSend.ready_time)
                        allTasksCopy[current_task_id - 1].wirelessSend.ready_time = parents_wireless_send_time;
                }
            }

            allTasksCopy[current_task_id - 1].wirelessSend.start_time = max(cloud_status[0], allTasksCopy[current_task_id - 1].wirelessSend.ready_time);
            allTasksCopy[current_task_id - 1].wirelessSend.finish_time = allTasksCopy[current_task_id - 1].wirelessSend.start_time + cloudTask.wireless_send_time;
            cloud_status[0] = allTasksCopy[current_task_id - 1].wirelessSend.finish_time;

            // Cloud compute stage
            int parent_finish_time_in_cloud = 0;
            for (int task_id : allTasksCopy[current_task_id - 1].parent_task_ids) {
                if (allTasksCopy[task_id - 1].cloudCompute.finish_time > parent_finish_time_in_cloud)
                    parent_finish_time_in_cloud = allTasksCopy[task_id - 1].cloudCompute.finish_time;
            }

            allTasksCopy[current_task_id - 1].cloudCompute.ready_time = max(allTasksCopy[current_task_id - 1].wirelessSend.finish_time, parent_finish_time_in_cloud);
            allTasksCopy[current_task_id - 1].cloudCompute.start_time = max(cloud_status[1], allTasksCopy[current_task_id - 1].cloudCompute.ready_time);
            allTasksCopy[current_task_id - 1].cloudCompute.finish_time = allTasksCopy[current_task_id - 1].cloudCompute.start_time + cloudTask.compute_time;
            cloud_status[1] = allTasksCopy[current_task_id - 1].cloudCompute.finish_time;

            // Wireless receive stage
            allTasksCopy[current_task_id - 1].wirelessReceive.ready_time = allTasksCopy[current_task_id - 1].cloudCompute.finish_time;
            allTasksCopy[current_task_id - 1].wirelessReceive.start_time = max(cloud_status[2], allTasksCopy[current_task_id - 1].wirelessReceive.ready_time);
            allTasksCopy[current_task_id - 1].wirelessReceive.finish_time = allTasksCopy[current_task_id - 1].wirelessReceive.start_time + cloudTask.wireless_receive_time;
            cloud_status[2] = allTasksCopy[current_task_id - 1].wirelessReceive.finish_time;
        }

        // Updating ready1 and ready2
        vector<int> core_sequence_ready_2;
        if (core_assigned == 0)
            core_sequence_ready_2 = sequenceCopy.core_1;
        else if (core_assigned == 1)
            core_sequence_ready_2 = sequenceCopy.core_2;
        else if (core_assigned == 2)
            core_sequence_ready_2 = sequenceCopy.core_3;
        else if (core_assigned == 3)
            core_sequence_ready_2 = sequenceCopy.cloud;

        int next_ready_2_update = -1;
        for (int i = 0; i < core_sequence_ready_2.size(); i++) {
            if (i == core_sequence_ready_2.size() - 1)
                break;
            if (core_sequence_ready_2[i] == current_task_id) {
                next_ready_2_update = core_sequence_ready_2[i + 1];
                break;
            }
        }

        next_ready_2_update -= 1;

        if (next_ready_2_update != -2)
            ready2[next_ready_2_update] = 0;

        // Updating ready1
        for (int i = 0; i < no_of_tasks; i++) {
            int parents_pending = 0;
            for (int task_id : allTasksCopy[i].parent_task_ids) {
                if (allTasksCopy[task_id - 1].is_scheduled == false)
                    parents_pending += 1;
            }
            ready1[i] = parents_pending;
        }

        // Updating the execution stack
        for (int i = 0; i < no_of_tasks; i++) {
            if (ready1[i] == 0 && ready2[i] == 0 && (allTasksCopy[i].is_scheduled == false) && !find_in_stack(toExecute, i + 1)) {
                toExecute.push(i + 1);
            }
        }
    }
}

// Function to create a new sequence after task reassignment
Sequence create_new_sequence(vector<Task>& allTasks, int target_task_id, int target_node, Sequence sequence) {
    int target_task_ready_time = 0;
    if (allTasks[target_task_id - 1].is_core == true)
        target_task_ready_time = allTasks[target_task_id - 1].localCompute.ready_time;
    if (allTasks[target_task_id - 1].is_core == false)
        target_task_ready_time = allTasks[target_task_id - 1].wirelessSend.ready_time;

    int current_node = allTasks[target_task_id - 1].core_assigned;

    // Remove the task from its current sequence
    if (current_node == 0) {
        sequence.core_1.erase(remove(sequence.core_1.begin(), sequence.core_1.end(), target_task_id), sequence.core_1.end());
    }
    if (current_node == 1) {
        sequence.core_2.erase(remove(sequence.core_2.begin(), sequence.core_2.end(), target_task_id), sequence.core_2.end());
    }
    if (current_node == 2) {
        sequence.core_3.erase(remove(sequence.core_3.begin(), sequence.core_3.end(), target_task_id), sequence.core_3.end());
    }
    if (current_node == 3) {
        sequence.cloud.erase(remove(sequence.cloud.begin(), sequence.cloud.end(), target_task_id), sequence.cloud.end());
    }

    vector<int> target_sequence;
    if (target_node == 0)
        target_sequence = sequence.core_1;
    else if (target_node == 1)
        target_sequence = sequence.core_2;
    else if (target_node == 2)
        target_sequence = sequence.core_3;
    else if (target_node == 3)
        target_sequence = sequence.cloud;

    vector<int> target_sequence_new;
    bool flag = false;

    for (int task_id : target_sequence) {
        if (target_node <= 2) {
            if (allTasks[task_id - 1].localCompute.start_time[target_node] < target_task_ready_time)
                target_sequence_new.push_back(task_id);

            if (allTasks[task_id - 1].localCompute.start_time[target_node] >= target_task_ready_time && flag == false) {
                target_sequence_new.push_back(target_task_id);
                flag = true;
            }
            if (allTasks[task_id - 1].localCompute.start_time[target_node] >= target_task_ready_time && flag == true) {
                target_sequence_new.push_back(task_id);
            }
        } else {
            if (allTasks[task_id - 1].wirelessSend.start_time < target_task_ready_time)
                target_sequence_new.push_back(task_id);

            if (allTasks[task_id - 1].wirelessSend.start_time >= target_task_ready_time && flag == false) {
                target_sequence_new.push_back(target_task_id);
                flag = true;
            }
            if (allTasks[task_id - 1].wirelessSend.start_time >= target_task_ready_time && flag == true) {
                target_sequence_new.push_back(task_id);
            }
        }
    }

    allTasks[target_task_id - 1].is_core = true;
    allTasks[target_task_id - 1].core_assigned = target_node;
    if (flag == false)
        target_sequence_new.push_back(target_task_id);

    // Update the sequence
    if (target_node == 0)
        sequence.core_1 = target_sequence_new;
    else if (target_node == 1)
        sequence.core_2 = target_sequence_new;
    else if (target_node == 2)
        sequence.core_3 = target_sequence_new;
    else if (target_node == 3) {
        sequence.cloud = target_sequence_new;
        allTasks[target_task_id - 1].is_core = false;
    }

    return sequence;
}

// Function to print task details
void print_tasks(vector<Task>& allTasks) {
    for (int i = 0; i < allTasks.size(); i++) {
        if (allTasks[i].is_core == true) {
            cout << "Task - " << i + 1 << " is assigned on core " << allTasks[i].core_assigned + 1 << " node :: ";
            cout << "Local Compute -> Ready Time: " << allTasks[i].localCompute.ready_time
                 << ", Start Time: " << allTasks[i].localCompute.start_time[allTasks[i].core_assigned]
                 << ", Finish Time: " << allTasks[i].localCompute.finish_time << endl;
        } else {
            cout << "Task - " << i + 1 << " is assigned on cloud :: ";
            cout << "Wireless Send -> Ready Time: " << allTasks[i].wirelessSend.ready_time
                 << ", Start Time: " << allTasks[i].wirelessSend.start_time
                 << ", Finish Time: " << allTasks[i].wirelessSend.finish_time << endl;
            cout << "Cloud Compute -> Ready Time: " << allTasks[i].cloudCompute.ready_time
                 << ", Start Time: " << allTasks[i].cloudCompute.start_time
                 << ", Finish Time: " << allTasks[i].cloudCompute.finish_time << endl;
            cout << "Wireless Receive -> Ready Time: " << allTasks[i].wirelessReceive.ready_time
                 << ", Start Time: " << allTasks[i].wirelessReceive.start_time
                 << ", Finish Time: " << allTasks[i].wirelessReceive.finish_time << endl;
        }
        cout << "=============================================================================" << endl;
    }
}
