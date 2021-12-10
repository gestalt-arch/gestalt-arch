using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
public class ControlButtons : MonoBehaviour
{
    private static Vector3 cube1PosIni;
    private static Vector3 cube2PosIni;
    private static Vector3 cube3PosIni;
    private static Vector3 cube1PosFin;
    private static Vector3 cube2PosFin;
    private static Vector3 cube3PosFin;
    public GameObject cube1;
    public GameObject cube2;
    public GameObject cube3;

    public GameObject kobuki1;
    public GameObject kobuki2;
    public GameObject kobuki3;
    StreamWriter sw;

    public void Reset()
    {
        cube1.transform.position = new Vector3(0f, 0.5f, 0f);
        cube2.transform.position = new Vector3(0f, 0.5f, 0f);
        cube3.transform.position = new Vector3(0f, 0.5f, 0f);
    }

    public void Initial()
    {
        //Get this to solver?
        cube1PosIni = cube1.transform.position;
        cube2PosIni = cube2.transform.position;
        cube3PosIni = cube3.transform.position;
    }

    public void Final()
    {
        cube1PosFin = cube1.transform.position;
        cube2PosFin = cube2.transform.position;
        cube3PosFin = cube3.transform.position;
    }

    public void Generate()
    {
        // save initial and final pos of "cube" to text file
        Vector3 kobuki1Pos = kobuki1.transform.position;
        Vector3 kobuki2Pos = kobuki2.transform.position;
        Vector3 kobuki3Pos = kobuki3.transform.position;

        sw = new StreamWriter("solver_params.txt", true);
        print("Saving cube ini, cube fin, kobuki pos");
        sw.WriteLine("{0} {1}\n", cube1PosIni[0], cube1PosIni[2]);
        sw.WriteLine("{0} {1}\n", cube2PosIni[0], cube2PosIni[2]);
        sw.WriteLine("{0} {1}\n", cube3PosIni[0], cube3PosIni[2]);
        sw.WriteLine("{0} {1}\n", cube1PosFin[0], cube1PosFin[2]);
        sw.WriteLine("{0} {1}\n", cube2PosFin[0], cube2PosFin[2]);
        sw.WriteLine("{0} {1}\n", cube3PosFin[0], cube3PosFin[2]);
        sw.WriteLine("{0} {1}\n", kobuki1Pos[0], kobuki1Pos[2]);
        sw.WriteLine("{0} {1}\n", kobuki2Pos[0], kobuki2Pos[2]);
        sw.WriteLine("{0} {1}\n", kobuki3Pos[0], kobuki3Pos[2]);

        sw.Flush();
        sw.Close();
        print("Saved");
    }

    public void Upload()
    {
        //upload solution from cpp to kobuki with BLE
    }
}
