using UnityEngine;
using UnityEngine.UI;

public class DebugPage : MonoBehaviour
{
    public Text InfoText;

    HandUpdate2 Hand_Update;

    // Start is called before the first frame update
    void Start()
    {
        GameObject obj = GameObject.Find("EventSystem");
        Hand_Update = obj.GetComponent<HandUpdate2>();
    }

    // Update is called once per frame
    void Update()
    {
        Quaternion Hand_Rotation = Hand_Update.Hand.rotation * Quaternion.Inverse(Hand_Update.Initial_q[0]);

        Debug.Log("Hand q.w: " + Hand_Rotation.w + ", q.x: " + Hand_Rotation.x + ", q.y: " + Hand_Rotation.y + ", q.z: " + Hand_Rotation.z);

        // Reference from https://discussions.unity.com/t/finding-pitch-roll-yaw-from-quaternions/65684/3, name: ratneshpatel response
        // Reference from https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
        float Pitch = -Mathf.Rad2Deg * Mathf.Atan2(2 * Hand_Rotation.x * Hand_Rotation.w - 2 * Hand_Rotation.y * Hand_Rotation.z, 1 - 2 * Hand_Rotation.x * Hand_Rotation.x - 2 * Hand_Rotation.z * Hand_Rotation.z);
        float Yaw = Mathf.Rad2Deg * Mathf.Atan2(2 * Hand_Rotation.y * Hand_Rotation.w - 2 * Hand_Rotation.x * Hand_Rotation.z, 1 - 2 * Hand_Rotation.y * Hand_Rotation.y - 2 * Hand_Rotation.z * Hand_Rotation.z);
        float Roll = -Mathf.Rad2Deg * Mathf.Asin(2 * Hand_Rotation.x * Hand_Rotation.y + 2 * Hand_Rotation.z * Hand_Rotation.w);

        InfoText.text = "Yaw: " + Yaw.ToString("F2") + "\nPitch: " + Pitch.ToString("F2") + "\nRoll: " + Roll.ToString("F2");
    }
}
