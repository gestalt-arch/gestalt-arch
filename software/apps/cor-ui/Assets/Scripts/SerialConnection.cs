using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

using System;
using System.IO.Ports;
// credit
// https://create.arduino.cc/projecthub/raisingawesome/unity-game-engine-and-arduino-serial-communication-12fdd5
// TODO: textbox to UI to type in COMPort

public class SerialConnection : MonoBehaviour
{
    SerialPort sp;
    //string COMPort = "COM3";
    string COMPort;
    string the_com = "";

    static Solver.PathStreamSolution path_stream_solution;

    private Solver.PathStream[] path_stream_vector = path_stream_solution.path_stream_vector;
    private uint num_path_streams = path_stream_solution.num_path_streams;

    // Use this for initialization
    void Start () {
    }

    // Update is called once per frame
    void Update() {
        COMPort = GameObject.Find("Main Camera/Canvas/Panel/COMInput/Text").GetComponent<Text>().text;
        if (string.IsNullOrEmpty(COMPort))
        {
            COMPort = "COM3";
        }
    }

    public void Upload() {

        the_com = COMPort;

        //foreach (string mysps in SerialPort.GetPortNames())
        //{
        //    print(mysps);
        //    if (mysps != COMPort) { the_com = mysps; break; } //take input from user
        //}
        sp = new SerialPort("\\\\.\\" + the_com, 115200); //big baud rate
        if (!sp.IsOpen)
        {
            print("Opening " + the_com + ", baud 115200");
            sp.Open();
            sp.ReadTimeout = 100;
            sp.Handshake = Handshake.None;
            if (sp.IsOpen) { print("Open ed sp"); }
        }


        if (sp.IsOpen) {
            // 2 (start) + 2 (num_bytes) + 1 (num_path_streams) + 1 (end) 
            // + num_path_streams((16*path_length_i) + 1)
            uint num_bytes = 6 + num_path_streams;
            for (uint i = 0; i < num_path_streams; i++) {
                num_bytes += path_stream_vector[i].path_length * 16;
            }

            byte[] content = new byte[num_bytes];

            print("Writing pog content over...");

            content[0] = (byte)'g';                                     // 'g' 's' to start
            content[1] = (byte)'s';
            content[2] = Convert.ToByte(num_bytes >> 16);        // upper total num bytes
            content[3] = Convert.ToByte(num_bytes & 0xffff);     // lower total num bytes
            content[4] = Convert.ToByte(num_path_streams);               // number of path streams

            uint counter = 5;
            for (uint i = 0; i < num_path_streams; i++) {
                uint path_length = path_stream_vector[i].path_length;
                // path length
                content[counter] = Convert.ToByte(path_length);
                // bot id
                content[counter + 1] = Convert.ToByte(path_stream_vector[i].bot_id);
                counter += 2;

                // x_pos_stream
                for (uint j = 0; j < path_length; j++) {
                    float x_cur = path_stream_vector[i].x_pos_stream[j];
                    byte[] bytearr = BitConverter.GetBytes(x_cur);
                    content[counter]     = bytearr[0];
                    content[counter + 1] = bytearr[1];
                    content[counter + 2] = bytearr[2];
                    content[counter + 3] = bytearr[3];
                    counter += 4;
                }

                // y_pos_stream
                for (uint j = 0; j < path_length; j++) {
                    float y_cur = path_stream_vector[i].y_pos_stream[j];
                    byte[] bytearr = BitConverter.GetBytes(y_cur);
                    content[counter] = bytearr[0];
                    content[counter + 1] = bytearr[1];
                    content[counter + 2] = bytearr[2];
                    content[counter + 3] = bytearr[3];
                    counter += 4;
                }

                // action_stream
                for (uint j = 0; j < path_length; j++) {
                    float action_cur = path_stream_vector[i].action_stream[j];
                    byte[] bytearr = BitConverter.GetBytes(action_cur);
                    content[counter] = bytearr[0];
                    content[counter + 1] = bytearr[1];
                    content[counter + 2] = bytearr[2];
                    content[counter + 3] = bytearr[3];
                    counter += 4;
                }

                // exclusion_stream
                for (uint j = 0; j < path_length; j++) {
                    float exclusion_cur = path_stream_vector[i].exclusion_stream[j];
                    byte[] bytearr = BitConverter.GetBytes(exclusion_cur);
                    content[counter] = bytearr[0];
                    content[counter + 1] = bytearr[1];
                    content[counter + 2] = bytearr[2];
                    content[counter + 3] = bytearr[3];
                    counter += 4;
                }
            }

            content[num_bytes - 1] = (byte) '\n';
            sp.Write(content, 0, Convert.ToInt32(num_bytes));
            
        } else if (!sp.IsOpen) {                                // open serial port
            sp.Open();
            print("Opened sp");
        }

    }

}
