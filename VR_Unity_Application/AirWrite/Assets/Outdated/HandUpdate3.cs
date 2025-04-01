using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HandUpdate3 : MonoBehaviour
{
    StorageManager SM;

    public Transform Hand;
    public Transform Thumb;
    public Transform Index;
    public Transform Middle;
    public Transform Ring;
    public Transform Pinky;

    public float thumb_pitch = 0;
    public float thumb_yaw = 0;

    public List<Quaternion> Initial_q = new List<Quaternion>();
    public List<Quaternion> Nodes_q = new List<Quaternion>();
    List<Transform> Nodes = new List<Transform>();

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
            Quaternion Hand_q = SM.World_To_Unity_Rotation(Storage.Gesture_Reset_Correct_q[0] * Storage.MahonyFilters[0].q);  // Get storage manager mahony filter q
            Debug.LogFormat("Storage MahonyFilter q: {0}, {1}, {2}, {3}", Hand_q.w, Hand_q.x, Hand_q.y, Hand_q.z);

            Hand.rotation = Hand_q * Initial_q[0];  // Calculate correct orientation on unity

            for (int index = 1; index < 5; index++)
            {
                // Base on Mahony filter algorithm update filter system q
                Quaternion node_q = SM.World_To_Unity_Rotation(Storage.Gesture_Reset_Correct_q[index] * Storage.MahonyFilters[index].q);
                Quaternion diff_hand_node_q = node_q * Quaternion.Inverse(Hand_q);

                if (index != 5)
                {
                    // Limit finger rotation direction and correct finger node
                    float diff_pitch = Get_Unity_Pitch(diff_hand_node_q) * Mathf.Rad2Deg;
                    diff_hand_node_q = Quaternion.AngleAxis((diff_pitch > 0) ? diff_pitch : 0, new Vector3(1, 0, 0));

                    // Correct quaterion first (Inverse rotation last time record), then rotate current base mahony quaternion. Finally recorver to normal initial rotation.
                    Nodes[index].rotation = diff_hand_node_q * Hand_q * Initial_q[index];
                    Nodes[index].transform.GetChild(0).rotation = diff_hand_node_q * diff_hand_node_q * Hand_q * Initial_q[index];
                    Nodes[index].transform.GetChild(0).GetChild(0).rotation = diff_hand_node_q * diff_hand_node_q * diff_hand_node_q * Hand_q * Initial_q[index];
                }
                else
                {
                    // Get difference between hand and thumb
                    diff_hand_node_q = new Quaternion(diff_hand_node_q.x, diff_hand_node_q.y, 0, diff_hand_node_q.w);
                    thumb_pitch = Get_Unity_Pitch(diff_hand_node_q) * Mathf.Rad2Deg;
                    thumb_yaw = Get_Unity_Yaw(diff_hand_node_q) * Mathf.Rad2Deg;

                    // Limit thumb rotation q and correct finger node (limit degree pitch : -20 ~ 20, yaw: -25 ~ 30)
                    Quaternion diff_hand_node_pitch_q = Quaternion.AngleAxis((thumb_pitch > -20) ? Mathf.Min(thumb_pitch, 40.0f) : -20, new Vector3(1, 0, 0));
                    Quaternion diff_hand_node_yaw_q = Quaternion.AngleAxis((thumb_yaw <= 35) ? Mathf.Max(thumb_yaw, -35.0f) : 35, new Vector3(0, 1, 0));
                    //Debug.Log("thumb yaw: " + thumb_yaw);

                    // Correct quaterion first (Inverse rotation last time record), then rotate current base mahony quaternion. Finally recorver to normal initial rotation.
                    Nodes[index].transform.rotation = diff_hand_node_pitch_q * diff_hand_node_yaw_q * Hand_q * Initial_q[index];
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
        Nodes_q.Add(Thumb.rotation);
        for (int node_index = 1; node_index < 6; node_index++)
        {
            if (node_index != 1)
            {
                Nodes_q.Add(Nodes[node_index].rotation);
                Nodes_q.Add(Nodes[node_index].transform.GetChild(0).rotation);
                Nodes_q.Add(Nodes[node_index].transform.GetChild(0).GetChild(0).rotation);
            }
        }
        return Nodes_q;
    }
}
