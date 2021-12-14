using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class CubePositions : MonoBehaviour
{
    private Text cubeText;
    private Vector3 cube1Pos;
    private Vector3 cube2Pos;
    private Vector3 cube3Pos;

    // Start is called before the first frame update
    void Start()
    {
        cubeText = GameObject.Find("Main Camera/Canvas/Panel/Locations").GetComponent<Text>();
    }

    // Update is called once per frame
    void Update()
    {
        cube1Pos = GameObject.Find("Cube 1").transform.position;
        cube2Pos = GameObject.Find("Cube 2").transform.position;
        cube3Pos = GameObject.Find("Cube 3").transform.position;
        cubeText.text = "Task Position 1: " + 
                        cube1Pos.ToString("F2") + "\nTask Position 2: " +
                        cube2Pos.ToString("F2") + "\nTask Position 3: " +
                        cube3Pos.ToString("F2");
    }
}
