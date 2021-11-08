using UnityEngine;
using System.Runtime.InteropServices;
using System;

public class SolverWrapper
{
    [StructLayout(LayoutKind.Sequential)]
    public struct PathStreamWrapper
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
        public float[] x_pos_stream;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
        public float[] y_pos_stream;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
        public int[] action_stream;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
        public int[] exclusion_stream;

        public uint path_length;

        public int bot_id;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct PathStreamSolutionWrapper
    {
        public IntPtr[] path_stream_vector;

        public uint num_path_streams;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct StateVectorWrapper
    {
        [MarshalAs(UnmanagedType.LPArray)]
        public float[] bot_x_pos_stream;

        [MarshalAs(UnmanagedType.LPArray)]
        public float[] bot_y_pos_stream;

        [MarshalAs(UnmanagedType.LPArray)]
        public int[] bot_ids;

        public uint num_bots;

        [MarshalAs(UnmanagedType.LPArray)]
        public float[] cube_x_pos_stream;

        [MarshalAs(UnmanagedType.LPArray)]
        public float[] cube_y_pos_stream;

        [MarshalAs(UnmanagedType.LPArray)]
        public int[] cube_ids;

        public uint num_cubes;
    }

    [DllImport("gestalt_solver")]
    public static extern uint solve_pathstream([In, Out] ref PathStreamSolutionWrapper path_stream_solution,
        [In] ref StateVectorWrapper initial_state,
        [In] ref StateVectorWrapper final_state
        );
}
