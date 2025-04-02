using System.IO;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
//using FileStream;
//using BinaryWriter;
using TMPro;

public class DynamicGestureMatch : MonoBehaviour
{
    public InputField InputFileName;
    public Dropdown FileNameList;
    public TextMeshProUGUI Time_Info;

    // Get current executing script (Hand_Update)
    HandUpdate Hand_Update;
    StorageManager SM;

    // Link Unity object
    public Transform Target_Hand;
    public Transform Target_Thumb;
    public Transform Target_Index;
    public Transform Target_Middle;
    public Transform Target_Ring;
    public Transform Target_Pinky;

    // Storage attitude of target hand node
    private List<Transform> Target_Nodes = new List<Transform>();
    private List<Quaternion> Nodes_q;

    // Declare thread to execute game start and record Game time
    Coroutine Record;
    Coroutine Play;
    private float start_time;
    private float ready_finish_time;
    private float record_finish_time;
    private float play_finish_time;

    // Setting game executing time, ready time, and score
    public int executing_time = 3;
    public int ready_time = 3;

    public float similar_threshold = 0.95f;
    public int similar_count = 0;
    public int similar_frame = 0;
    public int total_frame = 0;

    // Start is called before the first frame update
    void Start()
    {
        // Get current executing HandUpdate script
        GameObject obj = GameObject.Find("EventSystem");
        Hand_Update = obj.GetComponent<HandUpdate>();

        // Get current executing StorageManager script
        GameObject SM_obj = GameObject.Find("StorageManager");
        SM = obj.GetComponent<StorageManager>();

        // Add target node to List (for match)
        Target_Nodes.Add(Target_Hand);
        Target_Nodes.Add(Target_Pinky); Target_Nodes.Add(Target_Pinky.transform.GetChild(0)); Target_Nodes.Add(Target_Pinky.transform.GetChild(0).GetChild(0));
        Target_Nodes.Add(Target_Ring); Target_Nodes.Add(Target_Ring.transform.GetChild(0)); Target_Nodes.Add(Target_Ring.transform.GetChild(0).GetChild(0));
        Target_Nodes.Add(Target_Middle); Target_Nodes.Add(Target_Middle.transform.GetChild(0)); Target_Nodes.Add(Target_Middle.transform.GetChild(0).GetChild(0));
        Target_Nodes.Add(Target_Index); Target_Nodes.Add(Target_Index.transform.GetChild(0)); Target_Nodes.Add(Target_Index.transform.GetChild(0).GetChild(0));
        Target_Nodes.Add(Target_Thumb);

        // Get record gesture files on folder
        ListRecordFiles();

        // Declare status
        Record = null;
        Play = null;
    }

    // Update is called once per frame
    void Update()
    {
        // Poll record finish flag
        if ((Time.time > record_finish_time) && (Record != null))
        {
            FinishRecord();
            Time_Info.text = "Finish !";
        }

        // Poll play finish flag
        if ((Time.time > play_finish_time) && (Play != null))
        {
            FinishPlay();
            Time_Info.text = "Finish !";
            Time_Info.text = "Score: " + ((int)(((float)similar_frame / (float)total_frame) * 100f)).ToString();
        }
    }

    public void Dropdown_Value_Changed_Callback()
    {
        SM.Record_Btn_and_Event(0x06, (byte)(FileNameList.value + 1));
    }

    public void Forced_Dropdown_Value(int Dropdown_Index)
    {
        FileNameList.value = Dropdown_Index;
    }

    void ListRecordFiles()
    {
        // Search files on target folder
        string[] files = Directory.GetFiles("Gesture_Record");
        foreach (string f in files)
        {
            Debug.Log("filename: " + f);
            // Add filenames on list options
            List<Dropdown.OptionData> listOptions = new List<Dropdown.OptionData>();
            listOptions.Add(new Dropdown.OptionData(Path.GetFileName(f.ToString())));
            FileNameList.AddOptions(listOptions);
        }
    }

    public void StartRecord()
    {
        // If no set file name then notify user
        if (InputFileName.text == "")
        {
            Debug.Log("Please input file name");
            Time_Info.text = "Please input file name";
        }
        // If coroutine is null then start record process
        else if (Record == null)
        {
            // Setting start time, prepare time and record finish time
            start_time = Time.time;
            ready_finish_time = start_time + (float)ready_time;
            record_finish_time = start_time + (float)executing_time + (float)ready_time;

            // Open coroutine for record gesture
            Debug.Log("Start record gesture");
            Record = StartCoroutine(RecordGesture());
        }
    }

    public void FinishRecord()
    {
        // if coroutine doesn't stop, then stop it
        if (Record != null)
        {
            StopCoroutine(Record);
            Record = null;
            Debug.Log("Record Finish");
        }
    }

    IEnumerator RecordGesture()
    {
        // Prepare for user
        while (Time.time < ready_finish_time)
        {
            Time_Info.text = ((int)(ready_finish_time - Time.time)).ToString();
            Time_Info.text = (ready_finish_time - Time.time >= 0.5f) ? "Ready" : "Go !";

            yield return null;
        }

        // Setting file name and get gesture counter
        string filename = "Gesture_Record/" + InputFileName.text + ".txt";
        int Record_Frame_Counter_Start = Storage.Frame_Counter;

        // Add new file name on list option
        List<Dropdown.OptionData> listOptions = new List<Dropdown.OptionData>();
        listOptions.Add(new Dropdown.OptionData(InputFileName.text + ".txt"));
        FileNameList.AddOptions(listOptions);

        // Create file stream for writing file (binary file)
        FileStream fileStream = File.Open(filename, FileMode.Create);
        BinaryWriter binaryWriter = new BinaryWriter(fileStream);

        // Start record process
        while (Time.time < record_finish_time)
        {
            // Get current attitude on each node
            Nodes_q = Hand_Update.Get_Unity_Nodes_q();

            // Write pivot in beginning and data (14 nodes)
            binaryWriter.Write(12.34f);
            binaryWriter.Write(Storage.Frame_Counter - Record_Frame_Counter_Start);
            for (int index = 0; index < Nodes_q.Count; index++)
            {
                binaryWriter.Write(Nodes_q[index].w);
                binaryWriter.Write(Nodes_q[index].x);
                binaryWriter.Write(Nodes_q[index].y);
                binaryWriter.Write(Nodes_q[index].z);
            }

            // Notify reamain time for user
            Time_Info.text = "Time: " + ((int)(record_finish_time - Time.time)).ToString();
            yield return null;
        }
        fileStream.Close();
    }

    public void StartPlay()
    {
        // If user not select file then notify user to select
        if (FileNameList.options[FileNameList.value].text == "")
        {
            Debug.Log("Please select record file");
            Time_Info.text = "Please select record file";
        }
        // Start play gesture process
        else if (Play == null)
        {
            // Setting start time, prepare time and play finish time
            start_time = Time.time;
            ready_finish_time = start_time + (float)ready_time;
            play_finish_time = start_time + (float)executing_time + (float)ready_time;

            // Open coroutine for play recorded gesture
            Debug.Log("Start play gesture");
            Play = StartCoroutine(PlayGesture());
        }
    }

    public void FinishPlay()
    {
        // if coroutine doesn't stop, then stop it
        if (Play != null)
        {
            StopCoroutine(Play);
            Play = null;
            Debug.Log("Play Finish");
        }
    }

    IEnumerator PlayGesture()
    {
        // Prepare for user
        while (Time.time < ready_finish_time)
        {
            Time_Info.text = ((int)(ready_finish_time - Time.time)).ToString();
            Time_Info.text = (ready_finish_time - Time.time >= 0.5f) ? "Ready" : "Go !";

            yield return null;
        }

        // Declare play variable
        float pivot, w, x, y, z;
        int Play_Frame_Counter = 0;
        //int Current_Frame_Counter_Start = Storage.Frame_Counter;


        // Setting file name and creating file stream for reading data
        string filename = "Gesture_Record/" + FileNameList.options[FileNameList.value].text;
        FileStream fileStream = File.Open(filename, FileMode.Open);
        BinaryReader binaryReader = new BinaryReader(fileStream);

        while(Time.time < play_finish_time)
        {
            // Parse data from file stream
            pivot = binaryReader.ReadSingle();
            Play_Frame_Counter = binaryReader.ReadInt32();
            Nodes_q = Hand_Update.Get_Unity_Nodes_q();
            similar_count = 0;

            for (int index = 0; index < Target_Nodes.Count; index++)
            {
                if (binaryReader.BaseStream.Position != binaryReader.BaseStream.Length)
                {
                    w = binaryReader.ReadSingle();
                    x = binaryReader.ReadSingle();
                    y = binaryReader.ReadSingle();
                    z = binaryReader.ReadSingle();

                    Target_Nodes[index].rotation = new Quaternion(x, y, z, w);
                    //Debug.Log("Rotation: " + Target_Nodes[index].rotation);

                    if (Quaternion.Dot(Target_Nodes[index].rotation, Nodes_q[index]) > similar_threshold)
                        similar_count += 1;
                }
            }

            total_frame += 1;
            if (similar_count == Target_Nodes.Count)
                similar_frame += 1;

            Time_Info.text = "Time: " + ((int)(play_finish_time - Time.time + 1.0f)).ToString();
            yield return null;
        }
    }
}
