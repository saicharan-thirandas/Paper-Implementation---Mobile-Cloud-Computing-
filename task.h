#ifndef TASK_H
#define TASK_H

#include <vector>

using namespace std;

// Definition of the Task class
class Task {
public:
    // Task attributes
    int task_id;
    int core_speed[3];           // Speeds for cores 1, 2, and 3
    vector<int> parent_task_ids; // List of parent tasks
    vector<int> child_task_ids;  // List of child tasks

    // Computed values
    float weight_value;
    float priority_value;
    bool is_core;
    bool is_scheduled;

    // Local compute stage
    class LocalCompute {
    public:
        int ready_time;
        int start_time[3];
        int finish_time;
    } localCompute;

    // Wireless send stage
    class WirelessSend {
    public:
        int ready_time;
        int start_time;
        int finish_time;
    } wirelessSend;

    // Cloud compute stage
    class CloudCompute {
    public:
        int ready_time;
        int start_time;
        int finish_time;
    } cloudCompute;

    // Wireless receive stage
    class WirelessReceive {
    public:
        int ready_time;
        int start_time;
        int finish_time;
    } wirelessReceive;

    // Core assignment info
    int core_assigned;
};

// CloudTask class to represent cloud execution times and costs
class CloudTask {
public:
    int wireless_send_time;
    int compute_time;
    int wireless_receive_time;
    float compute_costs[3];
};

// CoreTask class to represent core execution costs
class CoreTask {
public:
    float compute_costs[3];
};

// Sequence class to store the task execution sequence for each core and cloud
class Sequence {
public:
    vector<int> core_1;
    vector<int> core_2;
    vector<int> core_3;
    vector<int> cloud;
};

#endif // TASK_H
