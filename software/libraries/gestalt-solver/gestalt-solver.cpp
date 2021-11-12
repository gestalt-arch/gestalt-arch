#include "gestalt-solver.h"
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>

/*
 * Returns Euclidean distance between two 2D vectors
 */
float GestaltSolver::get_2d_dist(float x1, float y1, float x2, float y2)
{
    return std::sqrtf(std::powf(x2-x1, 2) + std::powf(y2-y1, 2));
}

void GestaltSolver::dist_node_swap(DistNode** a, DistNode** b)
{
    DistNode* tmp = *a;
    *a = *b;
    *b = tmp;
}

void GestaltSolver::dist_node_sort(DistNode** dist_node_array, unsigned int num_dist_nodes)
{
    // A selection sort implementation
    int min_pos;
    for(int i = 0; i < num_dist_nodes - 1; i++) 
    {
        min_pos = i;
        for(int j = i + 1; j < num_dist_nodes; j++)
        {
            if(dist_node_array[j]->dist < dist_node_array[min_pos]->dist)
                min_pos = j;
        }
        dist_node_swap(&dist_node_array[min_pos], &dist_node_array[i]);
    }
}


/*
 * Solve the path streams and return the solution
 *  - returns num_path_streams
 *  - Populates path_stream_solution struct as OUTPUT
 *  - Takes inital_state and final_state vectors as INPUT
 */
unsigned int GestaltSolver::solve_pathstream(PathStreamSolution* path_stream_solution, StateVector* initial_state, StateVector* final_state)
{

    // Find distances between each bot in initial state
    // and each cube in the final state
    unsigned int num_dist_nodes = initial_state->num_objs * initial_state->num_bots;

    std::vector<std::shared_ptr<DistNode>> dist_node_array(num_dist_nodes);
    
    int idx = 0;
    for(unsigned int i = 0; i < initial_state->num_objs; i++)
    {
        for(unsigned int j = 0; j < initial_state->num_bots; j++)
        {
            std::shared_ptr<DistNode> d = std::make_shared<DistNode>();
            d->bot_id = initial_state->bot_ids[j];
            d->obj_id = initial_state->obj_ids[i];
            d->dist = get_2d_dist(
                initial_state->bot_x_pos[j],
                initial_state->bot_y_pos[j],
                initial_state->obj_x_pos[i],
                initial_state->obj_y_pos[i]
            );
            dist_node_array.push_back(d);
        }
    }

    // Sort the array of DistNodes by distance
    std::sort(dist_node_array.begin(), dist_node_array.end(),
        [](std::shared_ptr<DistNode> a, std::shared_ptr<DistNode> b) {
            return a->dist > b->dist;
        });

    // Assign each bot to an object
    //  Note this only works when num_bots = num_objs
    std::vector<std::shared_ptr<DistNode>> objective_array(initial_state->num_bots);
    int bot_idx = 0;
    while (bot_idx < initial_state->num_bots)
    {
        int cur_bot_id = initial_state->bot_ids[bot_idx];
        for (int i = 0; i < dist_node_array.size(); i++) {
            if (dist_node_array[i]->bot_id == cur_bot_id) {
                objective_array.push_back(dist_node_array[i]);
                bot_idx++;
                break;
            }
        }
    }
    dist_node_array.clear();

    return final_state->num_bots;
}