using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using System;
using System.IO.Ports;
//https://create.arduino.cc/projecthub/raisingawesome/unity-game-engine-and-arduino-serial-communication-12fdd5
// textbox to UI to type in COMPort

public class SerialConnection : MonoBehaviour
{
    SerialPort sp;
    float next_time; 
    int ii = 0;
    int COMPort = "COM3";
    // Use this for initialization
    void Start () {
        string the_com="";
        next_time = Time.time;
        
        foreach (string mysps in SerialPort.GetPortNames())
        {
            print(mysps);
            if (mysps != COMPort) { the_com = mysps; break; } //take input from user
        }
        sp = new SerialPort("\\\\.\\" + the_com, 115200); //something baud
        if (!sp.IsOpen)
        {
            print("Opening " + the_com + ", baud 115200");
            sp.Open();
            sp.ReadTimeout = 100;
            sp.Handshake = Handshake.None;
            if (sp.IsOpen) { print("Open"); }
        }
    }
    
    // Update is called once per frame
    void Update() {
        if (Time.time > next_time) { 
            if (!sp.IsOpen)
            {
                sp.Open();
                print("opened sp");
            }
            if (sp.IsOpen)
            {
                // serialize path stream function take pathstream struct into byte array
                // send over byte array
                
                // 1. number of path streams
                // 2. for loop and get length of each path
                // 3. bot id
                // 4. actual data with x y action and exclusion streams
                print("Writing " + ii);
                sp.Write((ii.ToString()));
            }
            next_time = Time.time + 5;
            if (++ii > 9) ii = 0;
        }
    }

}
