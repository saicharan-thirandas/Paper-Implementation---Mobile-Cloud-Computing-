#include "Scheduler.h"
#include <iostream>

using namespace std;

int main() {
    // Define tasks (example 3)
    Task task1 = {1, {9, 7, 5}, {}, {2, 3, 4, 5, 6}};
    Task task2 = {2, {8, 6, 5}, {1}, {8, 9}};
    Task task3 = {3, {6, 5, 4}, {1}, {7}};
    Task task4 = {4, {7, 5, 3}, {1}, {8, 9}};
    Task task5 = {5, {5, 4, 2}, {1}, {9}};
    Task task6 = {6, {7, 6, 4}, {1}, {8}};
    Task task7 = {7, {8, 5, 3}, {3}, {10}};
    Task task8 = {8, {6, 4, 2}, {2, 4, 6}, {10}};
    Task task9 = {9, {5, 3, 2}, {2, 4, 5}, {10}};
    Task task10 = {10, {7, 4, 2}, {7, 8, 9}, {12, 16}};
    Task task11 = {11, {9, 7, 5}, {7, 8, 9}, {13, 15, 17}};
    Task task12 = {12, {8, 6, 5}, {10}, {18}};
    Task task13 = {13, {6, 5, 4}, {11}, {19}};
    Task task14 = {14, {7, 5, 3}, {10}, {18}};
    Task task15 = {15, {5, 4, 2}, {11}, {19}};
    Task task16 = {16, {7, 6, 4}, {10}, {18}};
    Task task17 = {17, {8, 5, 3}, {11}, {19}};
    Task task18 = {18, {6, 4, 2}, {14, 16}, {20}};
    Task task19 = {19, {5, 3, 2}, {13, 15, 17}, {20}};
    Task task20 = {20, {7, 4, 2}, {18, 19}, {}};

    vector<Task> allTasks = {task1, task2, task3, task4, task5, task6, task7, task8, task9, task10,
                             task11, task12, task13, task14, task15, task16, task17, task18, task19, task20};

    int no_of_tasks = allTasks.size();

    CloudTask cloudTask = {3, 1, 1, {0.5, 0, 0}};
    int totalCloudTaskTime = 5;

    CoreTask coreTask = {{1, 2, 4}};

    // Primary assignment
    perform_primary_assignment(allTasks, totalCloudTaskTime);

    // Populate weight values
    populate_weight_values(allTasks, totalCloudTaskTime);

    // Populate priority values
    populate_priority_values(allTasks);

    // Sort tasks by priority
    sort(allTasks.begin(), allTasks.end(), SortByPriority());

    vector<int> priority_order;
    for (int i = 0; i < allTasks.size(); i++) {
        priority_order.push_back(allTasks[i].task_id);
    }

    // Restore task order by task ID
    sort(allTasks.begin(), allTasks.end(), SortByTaskId());

    // Execution unit selection
    Sequence sequence = execution_unit_selection(allTasks, priority_order, cloudTask);

    float E_init = compute_energy(allTasks, cloudTask, coreTask);
    float T_init = compute_time(allTasks);

    cout << "Initial energy: " << E_init << ", Initial time: " << T_init << endl;
    cout << endl;
    print_tasks(allTasks);

    float E_curr = 0;
    float T_curr = 0;

    int no_of_iterations = 100;
    for (int k = 0; k < no_of_iterations; k++) {
        cout << "====================================================================================" << endl;
        cout << " ***********************    Running iteration no " << k << "    ***********************" << endl;
        E_curr = compute_energy(allTasks, cloudTask, coreTask);
        T_curr = compute_time(allTasks);

        int resource_matrix[no_of_tasks][4] = {0};
        float energy_matrix[no_of_tasks][4] = {0};
        float time_matrix[no_of_tasks][4] = {0};

        for (int i = 0; i < sequence.core_1.size(); i++)
            resource_matrix[sequence.core_1[i] - 1][0] = 1;

        for (int i = 0; i < sequence.core_2.size(); i++)
            resource_matrix[sequence.core_2[i] - 1][1] = 1;

        for (int i = 0; i < sequence.core_3.size(); i++)
            resource_matrix[sequence.core_3[i] - 1][2] = 1;

        for (int i = 0; i < sequence.cloud.size(); i++)
            resource_matrix[sequence.cloud[i] - 1][3] = 1;

        for (int i = 0; i < no_of_tasks; i++) {
            for (int j = 0; j < 4; j++) {
                energy_matrix[i][j] = -1;
                time_matrix[i][j] = -1;

                if (resource_matrix[i][j] != 1) {
                    vector<Task> allTasksCopy = allTasks;
                    Sequence sequenceCopy = sequence;

                    sequenceCopy = create_new_sequence(allTasksCopy, i + 1, j, sequenceCopy);
                    kernel_function(allTasksCopy, sequenceCopy, cloudTask);

                    energy_matrix[i][j] = compute_energy(allTasksCopy, cloudTask, coreTask);
                    time_matrix[i][j] = compute_time(allTasksCopy);
                }
            }
        }

        float best_ratio = 0;
        int i_best = -1;
        int j_best = -1;

        for (int i = 0; i < no_of_tasks; i++) {
            for (int j = 0; j < 4; j++) {
                if (energy_matrix[i][j] != -1) {
                    if (time_matrix[i][j] > 1.5 * T_init)
                        continue;

                    float ratio = (E_curr - energy_matrix[i][j]) / abs(time_matrix[i][j] - T_curr + 0.00005);

                    if (ratio > best_ratio) {
                        best_ratio = ratio;
                        i_best = i;
                        j_best = j;
                    }
                }
            }
        }

        cout << "Optimal task and node for this iteration: Task " << i_best + 1 << ", Node " << j_best << endl;
        if (i_best != -1 && j_best != -1) {
            sequence = create_new_sequence(allTasks, i_best + 1, j_best, sequence);
            kernel_function(allTasks, sequence, cloudTask);
            print_tasks(allTasks);
        } else {
            cout << "No better solution found in this iteration." << endl;
        }
        cout << endl;
    }

    float E_final = compute_energy(allTasks, cloudTask, coreTask);
    float T_final = compute_time(allTasks);

    cout << "Final energy: " << E_final << ", Final time: " << T_final << endl;
    cout << endl;
    print_tasks(allTasks);

    return 0;
}
