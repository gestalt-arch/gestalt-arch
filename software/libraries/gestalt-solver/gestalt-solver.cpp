#include "gestalt-solver.h"

#include <iostream>
#include <fstream>

/*
 * Returns Euclidean distance between two 2D vectors
 */
float GestaltSolver::get_2d_dist(float x1, float y1, float x2, float y2)
{
    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
}

/*
 * Generate distance nodes
*/
std::vector<std::shared_ptr<GestaltSolver::DistNode>> GestaltSolver::gen_dist_nodes(StateVector *initial_state)
{
    // Find distances between each bot in initial state
    // and each cube in the final state
    unsigned int num_dist_nodes = initial_state->num_objs * initial_state->num_bots;

    std::vector<std::shared_ptr<DistNode>> dist_node_array(num_dist_nodes);

    int idx = 0;
    for (unsigned int i = 0; i < initial_state->num_objs; i++)
    {
        for (unsigned int j = 0; j < initial_state->num_bots; j++)
        {
            std::shared_ptr<DistNode> d = std::make_shared<DistNode>();
            d->bot_id = initial_state->bot_ids[j];
            d->obj_id = initial_state->obj_ids[i];
            d->dist = get_2d_dist(
                initial_state->bot_x_pos[j],
                initial_state->bot_y_pos[j],
                initial_state->obj_x_pos[i],
                initial_state->obj_y_pos[i]);
            dist_node_array.push_back(d);
        }
    }

    return dist_node_array;
}

/*
 * Solve the path streams and return the solution
 *  - returns num_path_streams
 *  - Populates path_stream_solution struct as OUTPUT
 *  - Takes inital_state and final_state vectors as INPUT
 */
unsigned int GestaltSolver::solve_pathstream(PathStreamSolution *path_stream_solution, StateVector *initial_state, StateVector *final_state, SolutionConfig *solution_config)
{
    /*
    std::ifstream readpositions("../apps/cor-ui/solve_params.txt");
    if (readpositions.is_open())
    {
        while (getline(readpositions, line))
        {
            cout << line << '\n';
        }
        myfreadpositionsile.close();
    }

    else
        cout << "Unable to open file";

    // Generate the dist nodes
    std::vector<std::shared_ptr<DistNode>>
        dist_node_array = gen_dist_nodes(initial_state);

    // Sort the array of DistNodes by distance
    std::sort(dist_node_array.begin(), dist_node_array.end(),
              [](std::shared_ptr<DistNode> a, std::shared_ptr<DistNode> b)
              {
                  return a->dist > b->dist;
              });

    // Assign each bot to an object
    // Note this only works when num_bots = num_objs
    std::vector<std::shared_ptr<DistNode>> objective_array(initial_state->num_bots);
    int bot_idx = 0;
    while (bot_idx < initial_state->num_bots)
    {
        int cur_bot_id = initial_state->bot_ids[bot_idx];
        for (int i = 0; i < dist_node_array.size(); i++)
        {
            if (dist_node_array[i]->bot_id == cur_bot_id)
            {
                objective_array.push_back(dist_node_array[i]);
                bot_idx++;
                break;
            }
        }
    }
    dist_node_array.clear();

    // write into file
    // 1. "initial" :[xi, yi]
    // "way points": [[x1, y1], [x2, y2], [xn, yn]].. new line
    // 2. "initial" :[xi, yi]
    // "way points": [[x1, y1], [x2, y2], [xn, yn]]

    std::ofstream myfile;
    myfile.open("pathstream.txt");
    if (myfile.is_open())
    {
        myfile << "Writing this to a file.\n\n";
        for (int b = 0; b < initial_state->num_bots; b++)
        {
            myfile << initial_state->bot_x_pos[b] << " ";
            myfile << initial_state->bot_y_pos[b] << std : endl;
            for (int l = 0; l < objective_array.size(); l++)
            {
                myfile << objective_array[l] << " ";
            }
            myfile << std : endl;
        }
    }
    else
        cout << "Unable to open file";

    myfile.close();

    // no more A*
    // create initial A* graph
    //AStarGraph init_graph = AStarGraph(STATE_GRAPH_DIM, solution_config->env_size, solution_config->bot_size, solution_config->obj_size);

    return final_state->num_bots;
    */
    return 0;
}

// GestaltSolver::AStarGraph::AStarGraph(int graph_dim, float env_size, float bot_size, float obj_size)
// {
//     this->bot_size = bot_size;
//     this->obj_size = obj_size;
//     float pos_increment = (float)graph_dim / env_size;
//     std::vector<std::vector<AStarNode> > all_nodes(graph_dim);
//     this->pos_node_map = std::unordered_map<Vector2, AStarNode>();
//     // Generate all nodes and populate pos_node_map
//     for (int r = 0; r < graph_dim; r++) {
//         all_nodes.push_back(std::vector<AStarNode>(graph_dim));
//         for (int c = 0; c < graph_dim; c++) {
//             Vector2 pos;
//             pos.x_pos = c * pos_increment;
//             pos.y_pos = r * pos_increment;

//             AStarNode node;
//             node.loc = pos;
//             node.occupied = false;

//             all_nodes[r].push_back(node);
//             this->pos_node_map.insert({ pos, node });
//         }
//     }
//     // Make all connections
//     for (int r = 0; r < graph_dim; r++) {
//         for (int c = 0; c < graph_dim; c++) {
//             AStarNode node = all_nodes[r][c];
//             if (r > 0) {
//                 if(c > 0)
//                     node.neighbors.push_back(all_nodes[r - 1][c - 1]);
//                 if(c < graph_dim - 1)
//                     node.neighbors.push_back(all_nodes[r - 1][c + 1]);
//                 node.neighbors.push_back(all_nodes[r - 1][c]);
//             }
//             if (r < graph_dim - 1) {
//                 if (c > 0)
//                     node.neighbors.push_back(all_nodes[r + 1][c - 1]);
//                 if (c < graph_dim - 1)
//                     node.neighbors.push_back(all_nodes[r + 1][c + 1]);
//                 node.neighbors.push_back(all_nodes[r + 1][c]);
//             }
//             //TODO: add middle left, middle right

//         }
//     }
// }

// std::vector<GestaltSolver::Vector2> GestaltSolver::AStarGraph::solve(Vector2 start_pos, Vector2 end_pos)
// {
//     return std::vector<Vector2>();
// }

// void main()
// {
//     PathStreamSolution *path_stream_solution;
//     StateVector *initial_state;
//     StateVector *final_state;
//     SolutionConfig *solution_config;
//     int ok = GestaltSolver::solve_pathstream(path_stream_solution, initial_state, final_state, solution_config);
// }