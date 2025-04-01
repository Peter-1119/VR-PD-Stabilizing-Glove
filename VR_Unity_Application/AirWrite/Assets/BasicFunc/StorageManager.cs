using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class StorageManager : MonoBehaviour
{
    public static StorageManager ins;
    public bool Global_Record_File_Open;
    public bool Play_Record;
    bool Play_Finish;

    /*   Connect Method   */
    public string port = "COM9";
    public int baudrate = 115200;
    byte[] buf = new byte[1024];
    byte[] pivot_buf = { 0xff, 0xfe, 0xfd, 0xfc };
    byte[] zero_buf = new byte[140];

    // IMU data
    public int Number_Of_IMU = 6;
    public bool Mahony_Update = false;

    // Global record data
    public TextMeshProUGUI Global_Record_Btn_Text;
    bool valid_data = false;
    string filename;
    FileStream fileStream;

    // Create file stream for writing file (binary file)
    FileStream PlayFileStream;
    BinaryReader PlayBinaryReader;
    Btn_Function BF;
    List<Action> FunctionsAction = new List<Action>();
    StaticGestureMatch SGM;
    DynamicGestureMatch DGM;
    System.DateTime LastTime;
    System.TimeSpan SampleRate = System.TimeSpan.FromMilliseconds(20);

    private void Awake()
    {
        if (ins == null)
        {
            ins = this;
            GameObject.DontDestroyOnLoad(gameObject);
        }
        else if (ins != this)
            Destroy(gameObject);
    }

    // Start is called before the first frame update
    void Start()
    {
        Global_Record_File_Open = false;

        for (int IMU_Index = 0; IMU_Index < Number_Of_IMU; IMU_Index++)
        {
            Storage.MahonyFilters[IMU_Index] = new Mahony();
            Storage.Gesture_Reset_Correct_q[IMU_Index] = new Quaternion(0, 0, 0, 1);  // Create reset rotation for each node
        }

        if (Play_Record == true)
        {
            Play_Finish = false;
            LastTime = System.DateTime.Now;

            // Get current executing HandUpdate script
            GameObject obj = GameObject.Find("EventSystem");
            BF = obj.GetComponent<Btn_Function>();

            FunctionsAction.Add(BF.Switch_To_Main_Page);
            FunctionsAction.Add(BF.Switch_To_Debug_Page);
            FunctionsAction.Add(BF.Switch_To_Air_Write_Page);
            FunctionsAction.Add(BF.Switch_To_Static_Gesture_Match_Page);
            FunctionsAction.Add(BF.Static_Gesture_Match_Start);
            FunctionsAction.Add(BF.Switch_To_Dynamic_Gesture_Match_Page);
            FunctionsAction.Add(BF.Play_Gesture);
            FunctionsAction.Add(Correct_Gestures_q);

            string filename = "Global_Gesture_Record_Test3.txt";
            PlayFileStream = File.Open(filename, FileMode.Open);
            PlayBinaryReader = new BinaryReader(PlayFileStream);
        }
    }

    // Update is called once per frame
    void Update()
    {
        valid_data = false;
        Storage.Frame_Counter++;
        // Start Record receive messages
        if (Storage.Global_Record == true && Global_Record_File_Open == false)
        {
            filename = "Global_Gesture_Record.txt";
            fileStream = File.Open(filename, FileMode.Create);
            Global_Record_Btn_Text.color = Color.red;
            Global_Record_File_Open = true;
            Debug.Log("Start global record gesture ...");
        }
        // Stop Record receive messages
        else if (Storage.Global_Record == false && Global_Record_File_Open == true)
        {
            fileStream.Close();
            Global_Record_Btn_Text.color = Color.black;
            Global_Record_File_Open = false;
            Debug.Log("Stop global record gesture ...");
        }

        if (Play_Record == true && !Play_Finish && (System.DateTime.Now - LastTime) > SampleRate)
        {
            LastTime += SampleRate;
            byte[] FileBuf = PlayBinaryReader.ReadBytes(140);

            if (FileBuf[3] == 0xfc)
            {
                valid_data = true;
                IMU_DataParse(FileBuf, (int)((byte)255 - FileBuf[0]));
                Mahony_Update = true;
            }
            else
            {
                if (FileBuf[4] == 0x00)
                {
                    FunctionsAction[(int)FileBuf[3]]();
                }
                else if (FileBuf[3] == 0x02 && FileBuf[4] == 0x01)
                    BF.Clear_Line_Renderer();
                else if (FileBuf[3] == 0x04 && FileBuf[4] != 0x00)
                {
                    GameObject SGM_obj = GameObject.Find("EventSystem");
                    SGM = SGM_obj.GetComponent<StaticGestureMatch>();
                    SGM.Game_Start_Play_Match_Gesture((int)FileBuf[4]);
                }
                else if (FileBuf[3] == 0x06 && FileBuf[4] != 0x00)
                {
                    GameObject DGM_obj = GameObject.Find("EventSystem");
                    DGM = DGM_obj.GetComponent<DynamicGestureMatch>();
                    DGM.Forced_Dropdown_Value((int)FileBuf[4]);
                }
            }
            if (PlayFileStream.Length == PlayFileStream.Position)
            {
                Debug.Log("Play Finish!");
                Play_Finish = true;
            }
        }

        if (Storage.Serial_Wire == false && Play_Record == false)
        {
            // Create serial port instance
            Storage.stream = new SerialPort(port, baudrate) { ReadTimeout = 1 };
            try
            {
                Storage.stream.Open(); // Open serial port
                Storage.Serial_Wire = true;
                Debug.Log("IMU SerialPort port was opened susccesfully!\n");
            }
            catch (IOException)
            {
                Debug.Log("No serial comport connect!\n");
            }
        }

        if (Storage.Serial_Wire && Play_Record == false)
        {
            // Reset buffer shift and buffer
            for (int i = 0; i < 256; i++) buf[i] = 0;

            try
            {
                // Read 180 bytes data from serial port
                Storage.stream.Read(buf, 0, 140);

                // if index 0 of the buffer is 0x0000 then drop
                if (buf[0] >= 0xfc && buf[1] != 0)
                {
                    valid_data = true;
                    IMU_DataParse(buf, (int)((byte)255 - buf[0]));
                    Mahony_Update = true;
                }
            }

            // Serial port reading timeout
            catch (TimeoutException) { /* Debug.Log("Receive data timeout."); */ }
            catch (IOException) { Storage.Serial_Wire = false; }
        }

        if (Global_Record_File_Open == true)
        {
            if (GameObject.Find("Global_Record_Btn") != null)
            {
                Button button = GameObject.Find("Global_Record_Btn").GetComponent<UnityEngine.UI.Button>();
                Global_Record_Btn_Text = button.GetComponentInChildren<TextMeshProUGUI>();
                Global_Record_Btn_Text.color = Color.red;
            }
            if (valid_data)
            {
                if (0xff - buf[0] > 0x00)
                    fileStream.Write(pivot_buf, 0, (int)(0xff - buf[0]));
                fileStream.Write(buf, 0, 140 - (int)(0xff - buf[0]));
            }
        }
        else
        {
            if (GameObject.Find("Global_Record_Btn") != null)
            {
                Button button = GameObject.Find("Global_Record_Btn").GetComponent<UnityEngine.UI.Button>();
                Global_Record_Btn_Text = button.GetComponentInChildren<TextMeshProUGUI>();
                Global_Record_Btn_Text.color = Color.black;
            }
        }
    }

    // Parse data to IMU orientation
    public void IMU_DataParse(byte[] data_buf, int buf_shift)
    {
        float q_w, q_x, q_y, q_z;
        for (int index = 0; index < Number_Of_IMU; index++)
        {
            q_w = BitConverter.ToSingle(data_buf, 23 - buf_shift + 16 * index);
            q_x = BitConverter.ToSingle(data_buf, 27 - buf_shift + 16 * index);
            q_y = BitConverter.ToSingle(data_buf, 31 - buf_shift + 16 * index);
            q_z = BitConverter.ToSingle(data_buf, 35 - buf_shift + 16 * index);

            Storage.MahonyFilters[index].q = new Quaternion(q_x, q_y, q_z, q_w);
        }
    }

    // Gesture_Reset_Correct_q
    public void Correct_Gestures_q()
    {
        for (int index = 0; index < Number_Of_IMU; index++)
            Storage.Gesture_Reset_Correct_q[index] = Quaternion.Inverse(Storage.MahonyFilters[index].q);
    }

    public Quaternion World_To_Unity_Rotation(Quaternion q)
    {
        return new Quaternion(q.x, q.z, q.y, q.w);
    }

    // Reset mahony update flag
    public void Reset_Mahony_Update_Flag()
    {
        Mahony_Update = false;
    }

    public void Record_Btn_and_Event(byte Btn_Index, byte Event_Index)
    {
        if (Global_Record_File_Open == true)
        {
            byte[] EventCode = { Btn_Index, Event_Index };
            fileStream.Write(pivot_buf, 0, 3);
            fileStream.Write(EventCode, 0, 2);
            fileStream.Write(zero_buf, 0, 135);
        }
    }

    public void Test()
    {
        Debug.Log("Hello World");
    }
}
