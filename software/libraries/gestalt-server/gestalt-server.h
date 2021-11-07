#include <stdio.h>

/* An individual path_stream, containing all path information for a single bot */
typedef struct PathStream
{
    float* x_pos_stream;
    float* y_pos_stream;
    int* exclusion_stream;
    unsigned int* path_length;
    int bot_id;
} PathStream;

/* A complete path_stream solution, containing all path_streams for all bots in the environment */
typedef struct PathStreamSolution
{
    PathStream* path_stream_vector;
    unsigned int* num_path_streams;
} PathStreamSolution;

typedef struct StateVector 
{
    float* bot_x_pos_stream;
    float* bot_y_pos_stream;
    int* bot_ids;
    unsigned int num_bots;
} StateVector;


PathStreamSolution* solve_pathstream(StateVector* initial_state, StateVector* final_state);



