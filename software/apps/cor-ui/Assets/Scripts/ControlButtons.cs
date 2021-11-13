using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ControlButtons : MonoBehaviour
{

    private Vector3 cube1PosIni;
    private Vector3 cube2PosIni;
    private Vector3 cube3PosIni;

    private Vector3 cube1PosFin;
    private Vector3 cube2PosFin;
    private Vector3 cube3PosFin;

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


}
