using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using System.Linq;

public class ControlButtons : MonoBehaviour
{
    public static int NUMBOTS = 3;
    private static Vector3[] cubePosIni = new Vector3[NUMBOTS];
    private static Vector3[] cubePosFin = new Vector3[NUMBOTS];
    private static Vector3[] kobukiPos = new Vector3[NUMBOTS];
    public GameObject cube1;
    public GameObject cube2;
    public GameObject cube3;

    public void Reset()
    {
        cube1.transform.position = new Vector3(0f, 0.5f, 0f);
        cube2.transform.position = new Vector3(0f, 0.5f, 0f);
        cube3.transform.position = new Vector3(0f, 0.5f, 0f);
    }

    public void Initial()
    {
        cubePosIni[0] = cube1.transform.position;
        cubePosIni[1] = cube2.transform.position;
        cubePosIni[2] = cube3.transform.position;
    }

    public void Final()
    {
        cubePosFin[0] = cube1.transform.position;
        cubePosFin[1] = cube2.transform.position;
        cubePosFin[2] = cube3.transform.position;
    }

    public void Generate()
    {
        // save initial and final pos of "cube" to text file
        kobukiPos[0] = kobuki1.transform.position;
        kobukiPos[1] = kobuki2.transform.position;
        kobukiPos[2] = kobuki3.transform.position;

        List<GestaltSolver.DistNode> solution = GestaltSolver.SolvePathstream(NUMBOTS, cubePosIni, cubePosFin, kobukiPos);
        GestaltSolver.SaveSolution("solution.txt", solution);


    }

    public void Upload()
    {
        //upload solution from cpp to kobuki with BLE
    }

    private void SolvePath()
    {

    }



}
