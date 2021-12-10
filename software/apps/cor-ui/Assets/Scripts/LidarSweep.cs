using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;

public class LidarSweep : MonoBehaviour
{
    float cur_deg;
    float DEGREES;
    float FRAMES;
    int num;
    StreamWriter sw;
    StreamWriter sw2;

    // Start is called before the first frame update
    void Start()
    {
        cur_deg = 0;
        DEGREES = 360;
        FRAMES = 420;
        num = 1;

    }

    // Update is called once per frame
    void Update()
    {
        if (Input.GetKeyDown(KeyCode.L))
        {
            string path = num + ".txt";
            sw = new StreamWriter("angle_" + path, true);
            sw2 = new StreamWriter("distance_" + path, true);

            RaycastHit hit;
            int i = 0;
            while (i < FRAMES)
            {
                print("Rotating...");
                if (Physics.Raycast(transform.position, transform.forward, out hit, 100.0f))
                {
                    transform.Rotate(0f, DEGREES / FRAMES, 0f);
                    cur_deg = cur_deg + DEGREES / FRAMES;

                    sw.WriteLine(cur_deg);
                    sw2.WriteLine(hit.distance);
                    i++;
                }
            }

            print("Rotated");

            num += 1;
            cur_deg = 0;
            sw.Flush();
            sw2.Flush();
            sw.Close();
            sw2.Close();
        }
    }
}

