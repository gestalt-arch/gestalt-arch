#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>
#include <unordered_map>

///////////////////////////////////////////////////////////////////////////////////
// OS DLL EXPORT definitions
#if defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS)
#define GESTALT_WINDOWS
#endif
#if defined(__linux__) || defined( __unix__)
#define GESTALT_LINUX
#endif

#ifdef GESTALT_LINUX
#define DLLEXPORT
#else
#define DLLEXPORT	__declspec(dllimport)
#endif
///////////////////////////////////////////////////////////////////////////////////

#define STATE_GRAPH_DIM 16 // Determines the dimension (M) of the state graph (M x M)
#define MAX_BOTS 16
#define MAX_OBJECTS 16

#define MAX_SOLUTION_LENGTH 128

#ifdef __cplusplus
extern "C" {
#endif
    class DLLEXPORT GestaltSolver 
    {

    public:
        /* An individual path_stream, containing all path information for a single bot */
        struct PathStream
        {
            float x_pos_stream[MAX_SOLUTION_LENGTH];
            float y_pos_stream[MAX_SOLUTION_LENGTH];
            int action_stream[MAX_SOLUTION_LENGTH];
            int exclusion_stream[MAX_SOLUTION_LENGTH];
            unsigned int path_length;
            int bot_id;
        };

        /* A complete path_stream solution, containing all path_streams for all bots in the environment */
        struct PathStreamSolution
        {
            PathStream* path_stream_vector[MAX_BOTS];
            unsigned int num_path_streams;
        };


        struct StateVector
        {
            float bot_x_pos[MAX_BOTS];
            float bot_y_pos[MAX_BOTS];
            int bot_ids[MAX_BOTS];
            int num_bots;
            float obj_x_pos[MAX_OBJECTS];
            float obj_y_pos[MAX_OBJECTS];
            int obj_ids[MAX_OBJECTS];
            int num_objs;
        };


        struct SolutionConfig
        {
            float env_size; // Length and width of the environment (in meters)
            float bot_size; // Size of a bot (in meters)
            float obj_size; // Size of an object (in meters)
        };

        /*
        * Returns Euclidean distance between two 2D vectors
        */
        static float get_2d_dist(float x1, float y1, float x2, float y2);

        /*
        * Solve the path streams and return the solution
        *  - returns num_path_streams
        *  - Populates path_stream_solution struct as OUTPUT
        *  - Takes inital_state and final_state vectors as INPUT
        */
        static unsigned int solve_pathstream(PathStreamSolution* path_stream_solution, StateVector* initial_state, StateVector* final_state, SolutionConfig* solution_config);

    private:

        struct Vector2 {
            float x_pos, y_pos;
        };

        class AStarGraph {
            public:
                
                struct AStarNode {
                    Vector2 loc;
                    bool occupied;
                    std::vector<AStarNode> neighbors;
                };

                AStarGraph(int graph_dim, float env_size, float bot_size, float obj_size);

                
                std::vector<Vector2> solve(Vector2 start_pos, Vector2 end_pos);

            private:
                float bot_size;
                float obj_size;
                std::unordered_map<Vector2, AStarNode> pos_node_map;

        };

        struct DistNode
        {
            int bot_id;
            int obj_id;
            float dist;
        };

        static std::vector<std::shared_ptr<DistNode>> gen_dist_nodes(StateVector* initial_state);

    };

#ifdef __cplusplus
}
#endif