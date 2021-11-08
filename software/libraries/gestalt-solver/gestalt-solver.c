#include "gestalt-solver.h"

/*
 * Allocate a path_stream with path_length number of entries in all streams
 */
void alloc_path_stream(PathStream* path_stream, unsigned int path_length)
{
    path_stream->x_pos_stream = (float*)malloc(sizeof(float)*path_length);
    path_stream->y_pos_stream = (float*)malloc(sizeof(float)*path_length);
    path_stream->action_stream = (int*)malloc(sizeof(int)*path_length);
    path_stream->exclusion_stream = (int*)malloc(sizeof(int)*path_length);
    path_stream->path_length = path_length;
}

/*
 * Allocate a StateVector with num_bots number of entries in bot streams and 
 * num_cubes number of entries in the cube streams
 */
void alloc_state_vector(StateVector* state_vector, unsigned int num_bots, unsigned int num_cubes)
{
    state_vector->bot_x_pos_stream = (float*)malloc(sizeof(float)*num_bots);
    state_vector->bot_y_pos_stream = (float*)malloc(sizeof(float)*num_bots);
    state_vector->bot_ids = (int*)malloc(sizeof(int)*num_bots);
    state_vector->num_bots = num_bots;
    state_vector->cube_x_pos_stream = (float*)malloc(sizeof(float)*num_cubes);
    state_vector->cube_y_pos_stream = (float*)malloc(sizeof(float)*num_cubes);
    state_vector->cube_ids = (int*)malloc(sizeof(int)*num_cubes);
    state_vector->num_cubes = num_cubes;
}

/*
 * Free a path_stream
 */
void free_path_stream(PathStream* path_stream)
{
    free(path_stream->x_pos_stream);
    free(path_stream->y_pos_stream);
    free(path_stream->action_stream);
    free(path_stream->exclusion_stream);
}

/*
 * Free a StateVector
 */
void free_state_vector(StateVector* state_vector)
{
    free(state_vector->bot_x_pos_stream);
    free(state_vector->bot_y_pos_stream);
    free(state_vector->bot_ids);
    free(state_vector->cube_x_pos_stream);
    free(state_vector->cube_y_pos_stream);
    free(state_vector->cube_ids);
}

/*
 * Returns Euclidean distance between two 2D vectors
 */
float get_2d_dist(float x1, float y1, float x2, float y2)
{
    return sqrtf(powf(x2-x1, 2) + powf(y2-y1, 2));
}

/*
 * Solve the path streams and return the solution
 *  - returns num_path_streams
 *  - Populates path_stream_solution struct as OUTPUT
 *  - Takes inital_state and final_state vectors as INPUT
 */
unsigned int solve_pathstream(PathStreamSolution* path_stream_solution, StateVector* initial_state, StateVector* final_state)
{
    // Naive solution - move each bot towards
    PathStream** path_stream_vector = (PathStream**)malloc(sizeof(PathStream*) * initial_state->num_bots);
    unsigned int path_length = 4;
    for(unsigned int i = 0; i < initial_state->num_bots; i++) 
    {
        path_stream_vector[i] = (PathStream*)malloc(sizeof(PathStream));
        alloc_path_stream(path_stream_vector[i], path_length);
    }


    PathStreamSolution* ps_solution = malloc(sizeof(struct PathStreamSolution));
    for(unsigned int i = 0; i < initial_state->num_cubes; i++) 
    {
        for(unsigned int j = 0; j < initial_state->num_bots; j++)
        {

        }
    }

    return NULL;
}