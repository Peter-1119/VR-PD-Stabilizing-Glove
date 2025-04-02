using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RotateTest : MonoBehaviour
{
    public float angleTest = 0;
    public int frameCount = 0;
    public Quaternion Initial_q;
    public Transform Hand;
    // Start is called before the first frame update
    void Start()
    {
        Initial_q = Hand.rotation;
        Hand.rotation = new Quaternion(0.0f, 0.0f, 0.0f, 1.0f) * Initial_q;  // left-hand rotation angle is positive
        //Hand.rotation = new Quaternion(0.707f, 0.0f, 0.0f, 0.707f);  // left-hand rotation angle is positive
    }

    // Update is called once per frame
    void Update()
    {
        frameCount += 1;
        if (frameCount > 1000)
        {
            angleTest += 1;
            Hand.rotation = Quaternion.AngleAxis(angleTest, new Vector3(1.0f, 0.0f, 0.0f)) * Initial_q;  // left-hand rotation angle is positive
        }
    }
}
