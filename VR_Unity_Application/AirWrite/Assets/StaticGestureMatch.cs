using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using TMPro;

public class StaticGestureMatch : MonoBehaviour
{
    // Link target hand and executing Hand_Update script
    //public TextMeshPro Time_Info;
    //public TextMeshPro Score_Info;
    public TextMeshProUGUI Time_Info;
    public TextMeshProUGUI Score_Info;
    HandUpdate Hand_Update;
    StorageManager SM;

    // Link Unity object
    public Transform Target_Hand_Proto;
    public Transform Target_Hand;
    public Transform Target_Thumb;
    public Transform Target_Index;
    public Transform Target_Middle;
    public Transform Target_Ring;
    public Transform Target_Pinky;

    // Storage attitude of target hand node
    private List<Transform> Target_Nodes = new List<Transform>();
    private List<Quaternion> Target_Nodes_q = new List<Quaternion>();
    private List<Quaternion> Nodes_q;

    // Declare thread to execute game start and record Game time
    Coroutine StartMatch;
    private float start_time;
    private float ready_finish_time;
    private float game_finish_time;

    // Setting game executing time, ready time, and score
    public int executing_time = 30;
    public int ready_time = 3;
    public float similar_score_threshold = 0.98f;
    private int score = 0;
    private int seed = 0;
    private int pre_seed = 0;

    // Setting some Gesture
    private List<Quaternion> Set_Target_q = new List<Quaternion>();

    // Start is called before the first frame update
    void Start()
    {
        // Get current executing HandUpdate script
        GameObject obj = GameObject.Find("EventSystem");
        Hand_Update = obj.GetComponent<HandUpdate>();

        // Get current executing StorageManager script
        GameObject SM_obj = GameObject.Find("StorageManager");
        SM = SM_obj.GetComponent<StorageManager>();

        // Add target node to List (for match)
        Target_Nodes.Add(Target_Hand);
        Target_Nodes.Add(Target_Pinky);
        Target_Nodes.Add(Target_Ring);
        Target_Nodes.Add(Target_Middle);
        Target_Nodes.Add(Target_Index);
        Target_Nodes.Add(Target_Thumb);

        // Add match gesture on system
        Set_Target_q.Add(Quaternion.identity);
        Set_Target_q.Add(Quaternion.AngleAxis(90, new Vector3(0, 0, 1)));
        Set_Target_q.Add(Quaternion.AngleAxis(-90, new Vector3(1, 0, 0)));
        Set_Target_q.Add(Quaternion.AngleAxis(-90, new Vector3(1, 0, 0)) * Quaternion.AngleAxis(90, new Vector3(0, 0, 1)));
        Set_Target_q.Add(Quaternion.AngleAxis(-90, new Vector3(1, 0, 0)) * Quaternion.AngleAxis(45, new Vector3(0, 0, 1)));

        StartMatch = null;
    }

    // Update is called once per frame
    void Update()
    {
        // Poll game finish flag
        if ((Time.time > game_finish_time) && (StartMatch != null))
        {
            FinishGame();
        }
    }

    public void StartGame()
    {
        // If coroutine is null, then start game
        if (StartMatch == null)
        {
            // Reset score and setting start time, prepare time and game finish time
            score = 0;
            start_time = Time.time;
            ready_finish_time = start_time + (float)ready_time;
            game_finish_time = start_time + (float)executing_time + (float)ready_time;
            Debug.Log("Game Start");
            //Debug.Log("Game Current Time: " + Time.time + ", ready_finish_time: " + ready_finish_time + ", game finish time: " + game_finish_time);

            // Start game process
            StartMatch = StartCoroutine(Hand_Match_Start());
        }
    }

    public void FinishGame()
    {
        // If coroutine doesn't sttp, then stop it
        if (StartMatch != null)
        {
            StopCoroutine(StartMatch);
            StartMatch = null;
            Debug.Log("Game Finish");
        }
    }

    IEnumerator Hand_Match_Start()
    {
        // Prepare for user
        while (Time.time < ready_finish_time)
        {
            //Time_Info.text = "Time: " + ((int)(ready_finish_time - Time.time)).ToString();
            Time_Info.text = (ready_finish_time - Time.time >= 0.5f) ? "Ready" : "Go !";

            yield return null;
        }

        if (Storage.Global_Play == false)
        {
            // Random pick match gesture
            seed = Random.Range(0, Set_Target_q.Count);
            pre_seed = seed;
            Target_Hand_Proto.rotation = Set_Target_q[seed];

            while (Time.time < game_finish_time)
            {
                // Match current gesture and match gesture
                if (Hand_Q_Match())
                {
                    // Increase core
                    score++;
                    Debug.Log("Match");

                    // Random pick next gesture
                    seed = Random.Range(0, Set_Target_q.Count - 1);
                    seed = (seed >= pre_seed) ? seed + 1 : seed;
                    pre_seed = seed;

                    Target_Hand_Proto.rotation = Set_Target_q[seed];
                    Score_Info.text = "Score: " + score.ToString();

                    SM.Record_Btn_and_Event(0x04, (byte)(seed + 1));
                    //SM.Test();
                }

                Time_Info.text = "Time: " + ((int)(game_finish_time - Time.time)).ToString();
                yield return null;
            }
        }
        else if(Storage.Global_Play == true)
        {
            while (Time.time < game_finish_time)
            {
                // Match current gesture and match gesture
                if (Hand_Q_Match())
                {
                    // Increase core
                    score++;
                    Debug.Log("Match");
                    Score_Info.text = "Score: " + score.ToString();
                }

                Time_Info.text = "Time: " + ((int)(game_finish_time - Time.time)).ToString();
                yield return null;
            }
        }
    }

    public void Game_Start_Play_Match_Gesture(int GestureSeed)
    {
        Target_Hand_Proto.rotation = Set_Target_q[GestureSeed - 1];
    }

   bool Hand_Q_Match()
    {
        // Clear all attitudes on list (clear history attitudes) and get current gesture
        float similar_count = 0;

        for (int node_index = 0; node_index < 6; node_index++)
        {
            float similar_score = Quaternion.Dot(Hand_Update.Nodes[node_index].rotation, Target_Nodes[node_index].rotation);
            if (similar_score >= similar_score_threshold)
                similar_count++;

            //Debug.Log("index " + node_index + " : " + similar_score + " - " + similar_count + " threshold: " + similar_score_threshold);
        }

        // Count match node
        if ((similar_count / Target_Nodes_q.Count) >= similar_score_threshold)
            return true;
        return false;
    }
}
