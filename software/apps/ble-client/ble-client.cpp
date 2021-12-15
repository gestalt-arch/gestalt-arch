#include <iostream>
#include <fstream>
#include "../../libraries/gestalt-solver/gestalt-solver.h"

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

    GestaltSolver::PathStreamSolution theSolution;
    theSolution.num_path_streams = numPathStreams;

    for (int i = 0; i < numPathStreams; i++)
    {
        GestaltSolver::PathStream curPathStream = theSolution.path_stream_vector[i];
        // for each pathstream, we save the following
        // botid
        input_file >> curPathStream.bot_id;
        cout << curPathStream.bot_id << "; ";

        // pathlength
        input_file >> curPathStream.path_length;
        cout << curPathStream.path_length << "; ";

        // array of x
        for (int j = 0; j < curPathStream.path_length; j++)
        {
            input_file >> curPathStream.x_pos_stream[j];
            cout << curPathStream.x_pos_stream[j] << "; ";
        }

        // array of y
        for (int j = 0; j < curPathStream.path_length; j++)
        {
            input_file >> curPathStream.y_pos_stream[j];
            cout << curPathStream.y_pos_stream[j] << "; ";
        }

        // array of action
        for (int j = 0; j < curPathStream.path_length; j++)
        {
            input_file >> curPathStream.action_stream[j];
            cout << curPathStream.action_stream[j] << "; ";
        }

        // array of exclusion
        for (int j = 0; j < curPathStream.path_length; j++)
        {
            input_file >> curPathStream.exclusion_stream[j];
            cout << curPathStream.exclusion_stream[j] << "; ";
        }
    }

    cout << endl;
    input_file.close();

    // do bluetooth GATT someone pls ty
    system("python3 testing.py");
    return EXIT_SUCCESS;
}