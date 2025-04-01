using System;
using System.IO;
using System.IO.Ports;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class StorageManager_ : MonoBehaviour
{
    public static StorageManager_ ins;
    public bool Global_Record;

    /*   Connect Method   */
    public bool Serial_Wire = false;
    public bool BLE = false;
    public bool Socket = false;

    // UART port name, baud rate and open status
    public string port = "COM9";
    public int baudrate = 115200;

    // Receive Buffer
    byte[] buf = new byte[1024];

    /*   BLE Connect Method   */
    public string Device_ID = "BluetoothLE#BluetoothLEb0:3c:dc:49:37:c9-00:80:e1:26:75:f8";
    public string Service_ID = "{00001234-cc7a-482a-984a-7f2ed5b3e58f}";
    public string Device_Characteristic_ID = "{00005678-8e22-4541-9d4c-21edae82ed19}";

    /*   Socket Connect Method   */
    private IPEndPoint ipEndPoint;
    private UdpClient udpClient;
    private Thread receiveThread;

    /*   Parse Data Parameters   */
    public bool Mahony_Update = false;

    // IMU data
    public int Number_Of_IMU = 6;
    public Quaternion Hand_q;

    // Global record data
    public TextMeshProUGUI Global_Record_Btn_Text;
    bool valid_data = false;
    string filename;
    FileStream fileStream;

    private void Awake()
    {
        if (ins == null)
        {
            ins = this;
            GameObject.DontDestroyOnLoad(gameObject);
        }
        else if(ins != this)
        {
            Destroy(gameObject);
        }
    }

    // Start is called before the first frame update
    void Start()
    {
        Storage.Serial_Wire = Serial_Wire;
        //Storage.BLE = BLE;
        //Storage.Socket = Socket;

        for (int IMU_Index = 0; IMU_Index < Number_Of_IMU; IMU_Index++)
        {
            Storage.MahonyFilters[IMU_Index] = new Mahony();
            Storage.Gesture_Reset_Correct_q[IMU_Index] = new Quaternion(0, 0, 0, 1);  // Create reset rotation for each node
        }

        if (Storage.Serial_Wire && (Storage.Serial_Wire_Open == false))
        {
            // Create serial port instance
            Storage.stream = new SerialPort(port, baudrate)
            {
                ReadTimeout = 1
            };
            Storage.stream.Open(); // Open serial port
            Storage.Serial_Wire_Open = true;
            Debug.Log("IMU SerialPort port was opened susccesfully!\n");
        }

        //else if(Storage.BLE && (Storage.BLE_Open == false))
        //{
        //    // Start scan surround device (to stabilize subscribe)
        //    BleApi.StartDeviceScan();
        //    BleApi.StopDeviceScan();

        //    // Start subscribe characteristic
        //    Debug.Log("Trying subscribe ...");
        //    BleApi.SubscribeCharacteristic(Device_ID, Service_ID, Device_Characteristic_ID, false);
        //    Storage.BLE_Open = true;
        //}
    }

    // Update is called once per frame
    void Update()
    {
        valid_data = false;
        Storage.Frame_Counter++;
        if (Storage.Global_Record == true && Global_Record == false)
        {
            filename = "Global_Gesture_Record.txt";
            fileStream = File.Open(filename, FileMode.Create);
            Global_Record_Btn_Text.color = Color.red;
            Global_Record = true;
            Debug.Log("Start global record gesture ...");
        }
        else if(Storage.Global_Record == false && Global_Record == true)
        {
            fileStream.Close();
            Global_Record_Btn_Text.color = Color.black;
            Global_Record = false;
            Debug.Log("Stop global record gesture ...");
        }

        if (Storage.Serial_Wire && Storage.Serial_Wire_Open)
        {
            // Reset buffer shift and buffer
            for (int i = 0; i < 256; i++) buf[i] = 0;

            try
            {
                // Read 180 bytes data from serial port
                Storage.stream.Read(buf, 0, 140);

                // if index 0 of the buffer is 0x0000 then drop
                if (buf[0] < 0xfc || buf[1] == 0)
                {
                    //Debug.Log("Data cannot be parse, cause pivot is 0.");
                }
                else
                {
                    valid_data = true;
                    IMU_DataParse(buf, (int)((byte)255 - buf[0]));
                    Hand_q = Storage.MahonyFilters[0].q;
                    Mahony_Update = true;
                    //Debug.Log(BitConverter.ToString(buf, 0, 180));
                    //Debug.LogFormat("qw: {0}, qx: {1}, qy: {2}, qz: {3}", Hand_q.w, Hand_q.x, Hand_q.y, Hand_q.z);
                }
            }

            // Serial port reading timeout
            catch (TimeoutException)
            {
                // Debug.Log("Receive data timeout.");
            }
        }

        //else if (Storage.BLE && Storage.BLE_Open)
        //{
        //    BleApi.BLEData res = new BleApi.BLEData();
        //    if (BleApi.PollData(out res, false))
        //    {
        //        IMU_DataParse(res.buf, 0);
        //        Hand_q = Storage.MahonyFilters[0].q;
        //        Mahony_Update = true;
        //        //Debug.Log(BitConverter.ToString(res.buf, 0, res.size));
        //        Debug.LogFormat("qw: {0}, qx: {1}, qy: {2}, qz: {3}", Hand_q.w, Hand_q.x, Hand_q.y, Hand_q.z);
        //    }
        //    //Debug.LogFormat("BLE status : {0}", res);
        //}

        if (Global_Record == true)
        {
            if (GameObject.Find("Global_Record_Btn") != null)
            {
                Button button = GameObject.Find("Global_Record_Btn").GetComponent<UnityEngine.UI.Button>();
                Global_Record_Btn_Text = button.GetComponentInChildren<TextMeshProUGUI>();
                Global_Record_Btn_Text.color = Color.red;
            }
            if (valid_data)
                fileStream.Write(buf, 0, 140);
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

            Debug.Log(Storage.MahonyFilters[index].q.w.ToString() + Storage.MahonyFilters[index].q.y.ToString());
            Storage.MahonyFilters[index].q = new Quaternion(q_x, q_y, q_z, q_w);
        }
    }

    // Gesture_Reset_Correct_q
    public void Correct_Gestures_q()
    {
        for (int index = 0; index < Number_Of_IMU; index++)
            Storage.Gesture_Reset_Correct_q[index] = Quaternion.Inverse(Storage.MahonyFilters[index].q);
        //Storage.Gesture_Reset_Correct_q[index] = Quaternion.Inverse(World_To_Unity_Rotation(Storage.MahonyFilters[index].q));
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
}
