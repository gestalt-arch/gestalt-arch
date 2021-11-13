using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DragCube : MonoBehaviour
{
    private Vector3 offset;

    void OnMouseDown()
    {
        offset = transform.position - GetHitPoint();
    }

    void OnMouseDrag()
    {
        transform.position = GetHitPoint() + offset;
        transform.position = new Vector3(transform.position.x, 0.5f, transform.position.z);
    }

    Vector3 GetHitPoint()
    {
        Plane plane = new Plane(Camera.main.transform.forward, transform.position);
        Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
        float dist;
        plane.Raycast(ray, out dist);
        return ray.GetPoint(dist);
    }

}