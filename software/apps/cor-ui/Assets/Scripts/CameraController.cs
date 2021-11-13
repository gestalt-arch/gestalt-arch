using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Linq;

public class CameraController : MonoBehaviour
{
    public bool lockUserInput = false;

    private float _cameraSpeed;
    public float _cameraSpeedNormal = 5.0f;
    public float _cameraSpeedMax = 10.0f;

    public float _speedH = 2.0f;
    public float _speedV = 2.0f;

    private float _yaw = 0.0f;
    private float _pitch = 0.0f;

    // Orbit controller
    public bool autoRotate = false;

    public Transform target;
    public Renderer targetRenderer;
    public float matAlpha;
    public float orbitDistance = 5.0f;
    public float orbitXSpeed = 120.0f;
    public float orbitYSpeed = 120.0f;

    public float orbitYMinLimit = -20f;
    public float orbitYMaxLimit = 80f;

    public float orbitDistanceMin = .5f;
    public float orbitDistanceMax = 15f;

    private Rigidbody _rigidbody;

    float x = 0.0f;
    float y = 0.0f;

    Vector3 wVector = Vector3.forward;
    Vector3 aVector = Vector3.left;
    Vector3 sVector = Vector3.back;
    Vector3 dVector = Vector3.right;

    public float forwardToCameraAngle;

    void SetCameraRotation(Quaternion rotation)
    {
        transform.rotation = rotation;
        _pitch = rotation.eulerAngles.x;
        _yaw = rotation.eulerAngles.y;
    }

    public static float ClampAngle(float angle, float min, float max)
    {
        if (angle < -360F)
            angle += 360F;
        if (angle > 360F)
            angle -= 360F;
        return Mathf.Clamp(angle, min, max);
    }

    // Start is called before the first frame update
    void Start()
    {
        Vector3 angles = transform.eulerAngles;
        x = angles.y;
        y = angles.x;

        ResetCameraTarget();
    }

    // Update is called once per frame
    void Update()
    {
       HandleInput();
    }
    void ResetCameraTarget()
    {
        RaycastHit hit;
        if (Physics.Raycast(transform.position, transform.forward, out hit, float.MaxValue, LayerMask.GetMask("Floor"), QueryTriggerInteraction.Ignore))
        {
            target.transform.position = hit.point;
            orbitDistance = hit.distance;
            Vector3 angles = transform.eulerAngles;
            x = angles.y;
            y = angles.x;
            UpdateCamPosition();
        }
        else
        {
            target.transform.position = transform.position + (transform.forward * 100);
            orbitDistance = 100;
            Vector3 angles = transform.eulerAngles;
            x = angles.y;
            y = angles.x;
            //UpdateCamPosition();
        }
    }

    private void HandleMouseInput()
    {
        // Orbit
        if (target)
        {
            // Rotate
            if (Input.GetMouseButton(1))
            {
                x += Input.GetAxis("Mouse X") * orbitXSpeed * orbitDistance * 0.02f;
                y -= Input.GetAxis("Mouse Y") * orbitYSpeed * orbitDistance * 0.02f;

                y = ClampAngle(y, orbitYMinLimit, orbitYMaxLimit);
                UpdateCamPosition();

                //_yaw += _speedH * Input.GetAxis("Mouse X");
                //_pitch -= _speedV * Input.GetAxis("Mouse Y");

                //transform.eulerAngles = new Vector3(_pitch, _yaw, 0.0f);
            }
        }
    }

    private void UpdateCamPosition()
    {
        Quaternion rotation = Quaternion.Euler(y, x, 0);
        RaycastHit hit;
        //Debug.DrawLine(target.position, transform.position, Color.red);
        if (Physics.Linecast(target.position, transform.position, out hit, LayerMask.GetMask("Floor"), QueryTriggerInteraction.Ignore))
        {
            orbitDistance -= hit.distance;
        }
        Vector3 negDistance = new Vector3(0.0f, 0.0f, -orbitDistance);
        Vector3 position = rotation * negDistance + target.position;
        UpdateTargetAlpha();

        SetCameraRotation(rotation);
        transform.position = position;
    }

    private void HandleInput()
    {
        if (!lockUserInput)
        {
            HandleLockableInput();
        }
        HandleNonLockableInput();
    }

    void HandleScrollWheelInput()
    {
        if (Input.GetAxis("Mouse ScrollWheel") != 0)
        {
            orbitDistance = Mathf.Clamp(orbitDistance - Input.GetAxis("Mouse ScrollWheel") * 20, orbitDistanceMin, orbitDistanceMax);
            UpdateCamPosition();
        }
    }

    void UpdateTargetAlpha()
    {
        matAlpha = (orbitDistance - orbitDistanceMin) / (orbitDistanceMax - orbitDistanceMin);
        matAlpha = Mathf.Max(Mathf.Sqrt(matAlpha) - 0.5f, 0);
        Color matColor = targetRenderer.material.color;
        matColor.a = matAlpha;
        targetRenderer.material.color = matColor;
    }

    void UpdateDirectionVectors()
    {
        Vector3 cameraToTarget = target.position - transform.position;
        cameraToTarget.y = 0;
        forwardToCameraAngle = Vector3.SignedAngle(Vector3.forward, cameraToTarget, Vector3.down);

        if (forwardToCameraAngle > -45f && forwardToCameraAngle <= 45f)
        {
            wVector = Vector3.forward;
            aVector = Vector3.left;
            sVector = Vector3.back;
            dVector = Vector3.right;
        }
        else if (forwardToCameraAngle > 45f && forwardToCameraAngle <= 135f)
        {
            wVector = Vector3.left;
            aVector = Vector3.back;
            sVector = Vector3.right;
            dVector = Vector3.forward;
        }
        else if (forwardToCameraAngle > 135f || forwardToCameraAngle <= -135f)
        {
            wVector = Vector3.back;
            aVector = Vector3.right;
            sVector = Vector3.forward;
            dVector = Vector3.left;
        }
        else if (forwardToCameraAngle > -135f && forwardToCameraAngle <= -45f)
        {
            wVector = Vector3.right;
            aVector = Vector3.forward;
            sVector = Vector3.left;
            dVector = Vector3.back;
        }

    }

    void HandleLockableInput()
    {

        HandleMouseInput();
        if (Input.GetKey(KeyCode.LeftShift))
        {
            _cameraSpeed = _cameraSpeedMax;
        }
        else
        {
            _cameraSpeed = _cameraSpeedNormal;
        }

        // TRANSLATIONAL MOVEMENT
        if (Input.GetKey(KeyCode.W) || Input.GetKey(KeyCode.UpArrow))
        {
            UpdateDirectionVectors();
            target.Translate(wVector * Time.deltaTime * _cameraSpeed);
            UpdateCamPosition();
        }
        if (Input.GetKey(KeyCode.A) || Input.GetKey(KeyCode.LeftArrow))
        {
            UpdateDirectionVectors();
            target.Translate(aVector * Time.deltaTime * _cameraSpeed);
            UpdateCamPosition();
        }
        if (Input.GetKey(KeyCode.S) || Input.GetKey(KeyCode.DownArrow))
        {
            UpdateDirectionVectors();
            target.Translate(sVector * Time.deltaTime * _cameraSpeed);
            UpdateCamPosition();
        }
        if (Input.GetKey(KeyCode.D) || Input.GetKey(KeyCode.RightArrow))
        {
            UpdateDirectionVectors();
            target.Translate(dVector * Time.deltaTime * _cameraSpeed);
            UpdateCamPosition();
        }
        if (Input.GetKey(KeyCode.Q))
        {
            target.Translate(Vector3.up * Time.deltaTime * _cameraSpeed);
            UpdateCamPosition();
        }
        if (Input.GetKey(KeyCode.E))
        {
            target.Translate(Vector3.down * Time.deltaTime * _cameraSpeed);
            UpdateCamPosition();
        }


    }

    void HandleNonLockableInput()
    {
        HandleScrollWheelInput();
    }

}
