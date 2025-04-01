using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HandUpdate : MonoBehaviour
{
    StorageManager SM;

    public Transform Hand;
    public Transform Thumb;
    public Transform Index;
    public Transform Middle;
    public Transform Ring;
    public Transform Pinky;

    public float World_thumb_diff_pitch = 0;
    public float World_thumb_diff_yaw = 0;
    public float World_thumb_diff_roll = 0;
    public float World_index_diff_pitch = 0;

    public List<Quaternion> Initial_q = new List<Quaternion>();
    public List<Quaternion> Nodes_q = new List<Quaternion>();
    public List<Transform> Nodes = new List<Transform>();

    // Start is called before the first frame update
    void Start()
    {
        // Get current executing HandUpdate script
        GameObject obj = GameObject.Find("StorageManager");
        SM = obj.GetComponent<StorageManager>();

        Nodes.Add(Hand); Initial_q.Add(Hand.rotation);
        Nodes.Add(Pinky); Initial_q.Add(Pinky.rotation);
        Nodes.Add(Ring); Initial_q.Add(Ring.rotation);
        Nodes.Add(Middle); Initial_q.Add(Middle.rotation);
        Nodes.Add(Index); Initial_q.Add(Index.rotation);
        Nodes.Add(Thumb); Initial_q.Add(Thumb.rotation);
    }

    // Update is called once per frame
    void Update()
    {
        if (SM.Mahony_Update)
        {
            SM.Reset_Mahony_Update_Flag();  // Reset storage manager update_flag

            Quaternion World_Hand_q = Storage.Gesture_Reset_Correct_q[0] * Storage.MahonyFilters[0].q;  // New Version
            Quaternion Hand_q = SM.World_To_Unity_Rotation(World_Hand_q);  // New Version
            //Debug.LogFormat("Storage MahonyFilter q: {0}, {1}, {2}, {3}", Hand_q.w, Hand_q.x, Hand_q.y, Hand_q.z);

            Hand.rotation = Hand_q * Initial_q[0];  // Calculate correct orientation on unity

            for (int index = 1; index < 6; index++)
            {
                // Base on Mahony filter algorithm update filter system q
                Quaternion World_node_q = Storage.Gesture_Reset_Correct_q[index] * Storage.MahonyFilters[index].q;
                Quaternion World_diff_hand_node_q = Quaternion.Inverse(World_Hand_q) * World_node_q;

                if (index != 5)
                {
                    // Limit finger rotation direction and correct finger node
                    float World_diff_pitch = Get_Body_Pitch(Quaternion.Inverse(World_diff_hand_node_q)) * Mathf.Rad2Deg;
                    Quaternion diff_hand_node_q = Quaternion.AngleAxis((World_diff_pitch < 0) ? -World_diff_pitch : 0, new Vector3(1, 0, 0));
                    if (index == 4)
                    {
                        World_index_diff_pitch = World_diff_pitch;
                        //Debug.Log("Pitch: " + World_diff_pitch);
                    }

                    // Correct quaterion first (Inverse rotation last time record), then rotate current base mahony quaternion. Finally recorver to normal initial rotation.
                    Nodes[index].rotation = Hand_q * diff_hand_node_q * Initial_q[index];
                    Nodes[index].transform.GetChild(0).rotation = Hand_q * diff_hand_node_q * diff_hand_node_q * Initial_q[index];
                    Nodes[index].transform.GetChild(0).GetChild(0).rotation = Hand_q * diff_hand_node_q * diff_hand_node_q * diff_hand_node_q * Initial_q[index];
                }
                else
                {
                    //// Get difference between hand and thumb
                    ////World_diff_hand_node_q = Quaternion.AngleAxis(-50f, new Vector3(0, 0, 1)) * World_diff_hand_node_q;
                    World_thumb_diff_pitch = Get_Body_Pitch(Quaternion.Inverse(World_diff_hand_node_q)) * Mathf.Rad2Deg;
                    World_thumb_diff_yaw = Get_Body_Yaw(Quaternion.Inverse(World_diff_hand_node_q)) * Mathf.Rad2Deg;
                    World_thumb_diff_roll = Get_Body_Roll(Quaternion.Inverse(World_diff_hand_node_q)) * Mathf.Rad2Deg;
                    //Debug.Log("Yaw: " + World_thumb_diff_yaw + ", Roll: " + World_thumb_diff_roll + ", Pitch: " + World_thumb_diff_pitch);

                    //// Limit thumb rotation q and correct finger node (limit degree pitch : -20 ~ 40, yaw: -25 ~ 30)
                    //Quaternion diff_hand_node_pitch_q = Quaternion.AngleAxis((-World_thumb_diff_pitch > -20) ? Mathf.Min(-World_thumb_diff_pitch, 30.0f) : -20, new Vector3(1, 0, 0));
                    //Quaternion diff_hand_node_yaw_q = Quaternion.AngleAxis((-World_thumb_diff_roll <= 10) ? Mathf.Max(-World_thumb_diff_roll, -25.0f) : 10, new Vector3(0, 1, 0));
                    ////Debug.Log("thumb yaw: " + thumb_yaw);

                    //// Correct quaterion first (Inverse rotation last time record), then rotate current base mahony quaternion. Finally recorver to normal initial rotation.
                    //Nodes[index].transform.GetChild(0).rotation = Hand_q * diff_hand_node_yaw_q * Initial_q[index];
                }
            }
        }
    }

    public float Get_Body_Pitch(Quaternion W2B_q)
    {
        // Get rotation angle base on x-axis according to right-hand
        return Mathf.Atan2(2.0f * (W2B_q.y * W2B_q.z + W2B_q.w * W2B_q.x), W2B_q.w * W2B_q.w - W2B_q.x * W2B_q.x - W2B_q.y * W2B_q.y + W2B_q.z * W2B_q.z);
    }

    public float Get_Body_Yaw(Quaternion W2B_q)
    {
        // Get rotation angle base on z-axis according to right-hand
        return Mathf.Atan2(2.0f * (W2B_q.x * W2B_q.y + W2B_q.w * W2B_q.z), W2B_q.w * W2B_q.w + W2B_q.x * W2B_q.x - W2B_q.y * W2B_q.y - W2B_q.z * W2B_q.z);
    }
    public float Get_Body_Roll(Quaternion W2B_q)
    {
        return Mathf.Asin(-2.0f * (W2B_q.x * W2B_q.z - W2B_q.w * W2B_q.y));
    }

    public float Get_Unity_Pitch(Quaternion q)
    {
        return Mathf.Atan2(2 * q.x * q.w - 2 * q.y * q.z, 1 - 2 * q.x * q.x - 2 * q.z * q.z);
    }


    public float Get_Unity_Yaw(Quaternion q)
    {
        return Mathf.Atan2(2 * q.y * q.w - 2 * q.x * q.z, 1 - 2 * q.y * q.y - 2 * q.z * q.z);
    }

    // Get current each node attitude
    public List<Quaternion> Get_Unity_Nodes_q()
    {
        Nodes_q.Clear();

        Nodes_q.Add(Hand.rotation);
        for (int node_index = 1; node_index < 5; node_index++)
        {
            Nodes_q.Add(Nodes[node_index].rotation);
            Nodes_q.Add(Nodes[node_index].transform.GetChild(0).rotation);
            Nodes_q.Add(Nodes[node_index].transform.GetChild(0).GetChild(0).rotation);
        }
        Nodes_q.Add(Thumb.rotation);

        return Nodes_q;
    }
}
