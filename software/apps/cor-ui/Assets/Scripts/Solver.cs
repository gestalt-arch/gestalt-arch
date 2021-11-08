using UnityEngine;
using System.Runtime.InteropServices;
using System;

public class Solver : MonoBehaviour
{
    public struct PathStream
    {
        public float[] x_pos_stream;
        public float[] y_pos_stream;
        public int[] action_stream;
        public int[] exclusion_stream;
        public uint path_length;
        public int bot_id;
    }

    public struct PathStreamSolution
    {
        public PathStream[] path_stream_vector;
        public uint num_path_streams;
    }

    public struct StateVector
    {
        public float[] bot_x_pos_stream;
        public float[] bot_y_pos_stream;
        public int[] bot_ids;
        public uint num_bots;
        public float[] cube_x_pos_stream;
        public float[] cube_y_pos_stream;
        public int[] cube_ids;
        public uint num_cubes;
    }

    public static PathStreamSolution GenerateSolution(StateVector initialState, StateVector finalState)
    {
        SolverWrapper.PathStreamSolutionWrapper pssWrapper = new SolverWrapper.PathStreamSolutionWrapper();
        SolverWrapper.StateVectorWrapper initialStateWrapper = new SolverWrapper.StateVectorWrapper();
        SolverWrapper.StateVectorWrapper finalStateWrapper = new SolverWrapper.StateVectorWrapper();

        initialStateWrapper.bot_x_pos_stream = initialState.bot_x_pos_stream;
        initialStateWrapper.bot_y_pos_stream = initialState.bot_y_pos_stream;
        initialStateWrapper.bot_ids = initialState.bot_ids;
        initialStateWrapper.num_bots = initialState.num_bots;
        initialStateWrapper.cube_x_pos_stream = initialState.cube_x_pos_stream;
        initialStateWrapper.cube_y_pos_stream = initialState.cube_y_pos_stream;
        initialStateWrapper.cube_ids = initialState.cube_ids;
        initialStateWrapper.num_cubes = initialState.num_cubes;

        finalStateWrapper.bot_x_pos_stream = finalState.bot_x_pos_stream;
        finalStateWrapper.bot_y_pos_stream = finalState.bot_y_pos_stream;
        finalStateWrapper.bot_ids = finalState.bot_ids;
        finalStateWrapper.num_bots = finalState.num_bots;
        finalStateWrapper.cube_x_pos_stream = finalState.cube_x_pos_stream;
        finalStateWrapper.cube_y_pos_stream = finalState.cube_y_pos_stream;
        finalStateWrapper.cube_ids = finalState.cube_ids;
        finalStateWrapper.num_cubes = finalState.num_cubes;


        SolverWrapper.solve_pathstream(ref pssWrapper, ref initialStateWrapper, ref finalStateWrapper);

        PathStream[] pathStreams = new PathStream[pssWrapper.num_path_streams];
        for(int i = 0; i < pssWrapper.num_path_streams; i++)
        {

        }

        //SolverWrapper.PathStreamWrapper pathStreamWrapper

        PathStreamSolution pss = new PathStreamSolution();
        


        return pss;
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
