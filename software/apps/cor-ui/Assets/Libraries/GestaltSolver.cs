using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Linq;
using System.IO;

public class GestaltSolver
{
    static int MAX_SOLUTION_LENGTH = 64;
    static StreamWriter sw;

    /* Distance node struct */
    public struct DistNode {
        public int botId;
        public int cubeId;
        public float dist;

        public DistNode(int botId, int cubeId, float dist) {
            this.botId = botId;
            this.cubeId = cubeId;
            this.dist = dist;
        }
    }

    public struct PathStream{
        float[] x_pos_stream;
        float[] y_pos_stream;
        int[] action_stream;     // all 0's. last one is 3
        int[] exclusion_stream;  // nothing
        int path_length;
        int bot_id;
    }


    public struct PathStreamSolution {
        PathStream[] path_stream_vector;
        int num_path_streams;
    }

    /*
    * Returns Euclidean distance between two 2D vectors
    */
    private static float GetDistance(float x1, float y1, float x2, float y2)
    {
        return Mathf.Sqrt(Mathf.Pow(x2 - x1, 2) + Mathf.Pow(y2 - y1, 2));
    }

    /*
    * Generate distance nodes
    */
    public static List<DistNode> SolvePathstream(int NUMBOTS, Vector3[] cubePosIni, Vector3[] cubePosFin, Vector3[] kobukiPos)
    {
        List<DistNode> distances = new List<DistNode>();
        for (int i = 0; i < 3; i++) // kobuki
        {
            for (int j = 0; j < 3; j++) // cube
            {
                distances.Add(new DistNode(i, j, 
                GetDistance(kobukiPos[i][0], kobukiPos[i][2],
                                cubePosIni[j][0], cubePosIni[j][2])));
            }
        }
        distances.Sort((s1, s2) => s1.dist.CompareTo(s2.dist));

        List<DistNode> solution = new List<DistNode>();
        List<int> cubesAssigned = new List<int>();
        int botIdx = 0;
        while (botIdx < NUMBOTS) {
            for (int i = 0; i < 9; i++) {
                if (distances[i].botId == botIdx && !cubesAssigned.Contains(distances[i].cubeId)) {
                    solution.Add(distances[i]);
                    cubesAssigned.Add(distances[i].cubeId);
                    botIdx++;
                    Debug.Log("Bot id: " + distances[i].botId + " Cube id: " + distances[i].cubeId +
                            " Distance: " + distances[i].dist );
                    break;
                }
            }
        }
        //
        return solution;
    }

    /* Save solution into given filepath */
    public static void SaveSolution(string fname, List<DistNode> solution) {
        sw = new StreamWriter(fname, true);
        Debug.Log("Saving solution");
        foreach (DistNode d in solution) {
            // write bot
            sw.WriteLine("");
        }
        // sw.WriteLine("{0} {1}\n", cubePosIni[0][0], cubePosIni[0][2]);
        // sw.WriteLine("{0} {1}\n", cubePosIni[1][0], cubePosIni[1][2]);
        // sw.WriteLine("{0} {1}\n", cubePosIni[2][0], cubePosIni[2][2]);
        // sw.WriteLine("{0} {1}\n", cubePosFin[0][0], cubePosFin[0][2]);
        // sw.WriteLine("{0} {1}\n", cubePosFin[1][0], cubePosFin[1][2]);
        // sw.WriteLine("{0} {1}\n", cubePosFin[2][0], cubePosFin[2][2]);
        // sw.WriteLine("{0} {1}\n", kobukiPos[0][0], kobukiPos[0][2]);
        // sw.WriteLine("{0} {1}\n", kobukiPos[1][0], kobukiPos[1][2]);
        // sw.WriteLine("{0} {1}\n", kobukiPos[2][0], kobukiPos[2][2]);

        sw.Flush();
        sw.Close();
        Debug.Log("Saved");
    }

    /* Depracated, save cube and kobuki positions */
    // public void SavePos() {
    //     sw = new StreamWriter("solver_params.txt", true);
    //     Debug.Log("Saving cube ini, cube fin, kobuki pos");
    //     // sw.WriteLine("{0} {1}\n", cubePosIni[0][0], cubePosIni[0][2]);
    //     // sw.WriteLine("{0} {1}\n", cubePosIni[1][0], cubePosIni[1][2]);
    //     // sw.WriteLine("{0} {1}\n", cubePosIni[2][0], cubePosIni[2][2]);
    //     // sw.WriteLine("{0} {1}\n", cubePosFin[0][0], cubePosFin[0][2]);
    //     // sw.WriteLine("{0} {1}\n", cubePosFin[1][0], cubePosFin[1][2]);
    //     // sw.WriteLine("{0} {1}\n", cubePosFin[2][0], cubePosFin[2][2]);
    //     // sw.WriteLine("{0} {1}\n", kobukiPos[0][0], kobukiPos[0][2]);
    //     // sw.WriteLine("{0} {1}\n", kobukiPos[1][0], kobukiPos[1][2]);
    //     // sw.WriteLine("{0} {1}\n", kobukiPos[2][0], kobukiPos[2][2]);

    //     sw.Flush();
    //     sw.Close();
    //     Debug.Log("Saved");
    // }
}
