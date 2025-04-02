using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AirWrite : MonoBehaviour
{
    // Declare how deep to draw line, and get objects LaserBean, LaserPoint
    public float Deep;
    public LineRenderer LaserBean;
    public LineRenderer line;
    public Transform LaserPoint;

    // Declare target position on wall(canvas)
    Vector3 EndPoint;
    List<GameObject> Lines = new List<GameObject>();

    // Declare thread and hand update script
    Coroutine drawing;
    HandUpdate Hand_Update;

    // Start is called before the first frame update
    void Start()
    {
        // Get current executing HandUpdate script
        GameObject obj = GameObject.Find("EventSystem");
        Hand_Update = obj.GetComponent<HandUpdate>();
    }

    // Update is called once per frame
    void Update()
    {
        // Get Hand rotation
        Quaternion Current_Hand_Rotation = Hand_Update.Hand.rotation * Quaternion.Inverse(Hand_Update.Initial_q[0]);
        float pitch = Hand_Update.Get_Unity_Pitch(Current_Hand_Rotation);
        float yaw = Hand_Update.Get_Unity_Yaw(Current_Hand_Rotation);

        // Find the destination for emitting laser from index node to wall(canvas)
        float EndPoint_x = Mathf.Asin(yaw) * (Deep - Hand_Update.Index.position.z);
        float EndPoint_y = -Mathf.Asin(pitch) * (Deep - Hand_Update.Index.position.z);
        EndPoint = new Vector3(EndPoint_x, EndPoint_y, 20);

        // Update LaserBean and LaserPoint
        LaserBean.SetPosition(0, Hand_Update.Index.position);
        LaserBean.SetPosition(1, EndPoint);
        LaserPoint.position = EndPoint;

        // If thumb close then draw line (yaw exceed than 18 degree
        if (Hand_Update.World_index_diff_pitch >= -15.0f)
        {
            StartLine();
        }
        else
        {
            FinishLine();
        }
    }

    void StartLine()
    {
        if (drawing == null)
        {
            Debug.Log("Create draw");
            drawing = StartCoroutine(HandDrawLine());
        }
    }

    void FinishLine()
    {
        if (drawing != null)
        {
            StopCoroutine(drawing);
            Debug.Log("Stop draw");
            drawing = null;
        }
    }

    // Reference youtube: https://www.youtube.com/watch?time_continue=135&v=bcPNyY84qAY&embeds_referring_euri=https%3A%2F%2Fwww.google.com%2Fsearch%3Fq%3Dunity%2Bclick%2Bmouse%2Band%2Bdraw%26sca_esv%3Dfd299e0f5b2672c9%26sxsrf%3DACQVn0-8hS8Dh3HNGZd8LYhYwz8L2KX31Q%253A1&source_ve_path=Mjg2NjY&feature=emb_logo
    IEnumerator HandDrawLine()
    {
        // Create LineRenderer object to asset
        GameObject newGameObject = Instantiate(Resources.Load("Line") as GameObject, new Vector3(0, 0, 0), Quaternion.identity);
        line = newGameObject.GetComponent<LineRenderer>();
        Lines.Add(newGameObject);
        line.positionCount = 0;

        while(true)
        {
            // Add new point on LineRenderer
            line.positionCount++;
            line.SetPosition(line.positionCount - 1, EndPoint - new Vector3 (0, 0, 0.5f));
            //Debug.Log("EndPoint x: " + EndPoint.x + ", y: " + EndPoint.y + ", z: " + EndPoint.z);

            yield return null;
        }
    }

    public void Clear_Line_Renderer()
    {
        for(int LineIndex = 0; LineIndex < Lines.Count; LineIndex++)
        {
            if (drawing == null || (LineIndex != Lines.Count - 1))
            {
                GameObject.Destroy(Lines[0]);
                Lines.RemoveAt(0);
            }
            else
                line.positionCount = 0;
        }
    }
}
