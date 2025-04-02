using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class UnityQTest : MonoBehaviour
{
    public Transform Hand;
    public bool xRotation = false;


    // Start is called before the first frame update
    void Start()
    {
        Hand.rotation = new Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    }

    // Update is called once per frame
    void Update()
    {
        if (xRotation)
            Hand.rotation = new Quaternion(0.707f, 0.0f, 0.0f, 0.707f);
        else
            Hand.rotation = new Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    }
}
