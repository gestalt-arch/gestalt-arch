using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Linq;
using System.IO;

public class GestaltSolver
{
    // static int MAX_SOLUTION_LENGTH = 64;
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
        public int botId;
        public int pathLength; // initial, 1st waypt, 2nd waypt, home: lengh = 4
        public float[] xPosStream;
        public float[] yPosStream;
        public int[] actionStream;     // all 0's. last one is 3
        public int[] exclusionStream;  // nothing

        public PathStream(int botId, int pathLength, float[] xPosStream, 
                            float[] yPosStream,int[] actionStream, int[] exclusionStream) {
            this.botId           = botId;
            this.pathLength      = pathLength;
            this.xPosStream      = xPosStream;
            this.yPosStream      = yPosStream;
            this.actionStream    = actionStream;
            this.exclusionStream = exclusionStream;
        }      
    }


    public struct PathStreamSolution {
        public PathStream[] pathStreamVector;
        public int numPathStream; // NUMBOTS

        public PathStreamSolution(PathStream[] pathStreamVector, int numPathStream) {
            this.pathStreamVector = pathStreamVector;
            this.numPathStream = numPathStream;
        }
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
    public static PathStreamSolution SolvePathstream(int NUMBOTS, Vector3[] cubePosIni, Vector3[] cubePosFin, Vector3[] kobukiPos)
    {
        List<DistNode> distances = new List<DistNode>();
        for (int i = 0; i < NUMBOTS; i++) // kobuki
        {
            for (int j = 0; j < NUMBOTS; j++) // cube
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
            for (int i = 0; i < NUMBOTS * NUMBOTS; i++) {
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
        //  bot idx will go low to high
        PathStream[] pathStreamVector = new PathStream[NUMBOTS];
        int pathLength = 4;
        for (int i = 0; i < NUMBOTS; i++) {
            //int botId, int pathLength, float[] xPosStream, 
            //float[] yPosStream,int[] actionStream, int[] exclusionStream
            int curCb = solution[i].cubeId;
            float[] xPosStream      = new float[] {kobukiPos[i][0], cubePosIni[curCb][0], cubePosFin[curCb][0], kobukiPos[i][0]};
            float[] yPosStream      = new float[] {kobukiPos[i][2], cubePosIni[curCb][2], cubePosFin[curCb][2], kobukiPos[i][2]};
            int[] actionStream      = new int[] {0, 0, 0, 3};
            int[] exclusionStream   = new int[] {0, 0, 0, 0};
            pathStreamVector[i] = new PathStream(i, pathLength, xPosStream, 
                            yPosStream, actionStream, exclusionStream);
        }

        return new PathStreamSolution(pathStreamVector, NUMBOTS);        
    }

    /* Save solution into given filepath */
    public static void SaveSolution(string fname, PathStreamSolution solution) {
        sw = new StreamWriter(fname, true);

        Debug.Log("Saving solution into " + fname);
        // number of pathstreams to read for
        sw.WriteLine(solution.numPathStream);
        foreach (PathStream p in solution.pathStreamVector) {
            // for each pathstream, we save the following
            // botid
            // pathlength
            // array of x
            // array of y
            // array of action
            // array of exclusion
            sw.WriteLine(p.botId);
            sw.WriteLine(p.pathLength);

            foreach (float f in p.xPosStream) {
                sw.Write(f + " ");
            }
            sw.WriteLine();

            foreach (float f in p.yPosStream) {
                sw.Write(f + " ");
            }
            sw.WriteLine();

            foreach (int i in p.actionStream) {
                sw.Write(i + " ");
            }
            sw.WriteLine();

            foreach (int i in p.exclusionStream) {
                sw.Write(i + " ");
            }
            sw.WriteLine();
        }
        
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
