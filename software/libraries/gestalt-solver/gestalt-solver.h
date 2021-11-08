#include <stdio.h>
#include <stdlib.h>
#include <math.h>


/* An individual path_stream, containing all path information for a single bot */
typedef struct PathStream
{
    float* x_pos_stream;
    float* y_pos_stream;
    int* action_stream;
    int* exclusion_stream;
    unsigned int path_length;
    int bot_id;
} PathStream;

/* A complete path_stream solution, containing all path_streams for all bots in the environment */
typedef struct PathStreamSolution
{
    PathStream** path_stream_vector;
    unsigned int num_path_streams;
} PathStreamSolution;

typedef struct StateVector 
{
    float* bot_x_pos_stream;
    float* bot_y_pos_stream;
    int* bot_ids;
    unsigned int num_bots;
    float* cube_x_pos_stream;
    float* cube_y_pos_stream;
    int* cube_ids;
    unsigned int num_cubes;
} StateVector;

typedef struct DistNode
{
    int bot_id;
    int cube_id;
    float dist;
} DistNode;


/*
 * Allocate a PathStream with path_length number of entries in all streams
 */
void alloc_path_stream(PathStream* path_stream, unsigned int path_length);

/*
 * Allocate a StateVector with num_bots number of entries in bot streams and 
 * num_cubes number of entries in the cube streams
 */
void alloc_state_vector(StateVector* state_vector, unsigned int num_bots, unsigned int num_cubes);

/*
 * Free a PathStream
 */
void free_path_stream(PathStream* path_stream);

/*
 * Free a StateVector
 */
void free_state_vector(StateVector* state_vector);

/*
 * Returns Euclidean distance between two 2D vectors
 */
float get_2d_dist(float x1, float y1, float x2, float y2);

void dist_node_swap(DistNode** a, DistNode** b);

void dist_node_sort(DistNode** dist_node_array, unsigned int num_dist_nodes);

/*
 * Solve the path streams and return the solution
 *  - returns num_path_streams
 *  - Populates path_stream_solution struct as OUTPUT
 *  - Takes inital_state and final_state vectors as INPUT
 */
unsigned int solve_pathstream(PathStreamSolution* path_stream_solution, StateVector* initial_state, StateVector* final_state);



