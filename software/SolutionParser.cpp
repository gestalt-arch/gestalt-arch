#include <iostream>
#include <fstream>
extern "C"
{
#include "gestalt-solver.h"
}

using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;

int main()
{
    string filename("solution.txt");
    ifstream input_file(filename);
    if (!input_file.is_open()) // if can't open file..
    {
        cerr << "Could not open the file - '"
             << filename << "'" << endl;
        return EXIT_FAILURE;
    }

    //float a, b, c, d, e, f;
    //myfile >> a >> b >> c >> d >> e >> f;

    int numPathStreams; // get num path streams first
    input_file >> numPathStreams;
    cout << numPathStreams << "; ";

    PathStreamSolution theSolution;
    theSolution->num_path_streams = numPathStreams;

    for (int i = 0; i < numPathStreams; i++)
    {
        PathStream curPathStream = theSolution->path_stream_vector[i];
        // for each pathstream, we save the following
        // botid
        input_file >> curPathStream->bot_id;

        // pathlength
        input_file >> curPathStream->path_length;

        // array of x
        for (int j = 0; j < curPathStream->path_length; j++)
        {
            input_file >> curPathStream->x_pos_stream[j];
        }

        // array of y
        for (int j = 0; j < curPathStream->path_length; j++)
        {
            input_file >> curPathStream->y_pos_stream[j];
        }

        // array of action
        for (int j = 0; j < curPathStream->path_length; j++)
        {
            input_file >> curPathStream->action_stream[j];
        }

        // array of exclusion
        for (int j = 0; j < curPathStream->path_length; j++)
        {
            input_file >> curPathStream->exclusion_stream[j];
        }
    }

    /* An individual path_stream, containing all path information for a single bot */
    // struct PathStream
    // {
    //     float x_pos_stream[MAX_SOLUTION_LENGTH];
    //     float y_pos_stream[MAX_SOLUTION_LENGTH];
    //     int action_stream[MAX_SOLUTION_LENGTH];
    //     int exclusion_stream[MAX_SOLUTION_LENGTH];
    //     unsigned int path_length;
    //     int bot_id;
    // };

    // /* A complete path_stream solution, containing all path_streams for all bots in the environment */
    // struct PathStreamSolution
    // {
    //     PathStream path_stream_vector[MAX_BOTS];
    //     unsigned int num_path_streams;
    // };

    cout << endl;
    input_file.close();

    return EXIT_SUCCESS;
}