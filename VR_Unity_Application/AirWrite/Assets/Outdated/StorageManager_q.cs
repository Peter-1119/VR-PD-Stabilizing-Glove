using System;
using System.IO.Ports;
using UnityEngine;
using System.Net;
using System.Net.Sockets;
using System.Threading;

public class StorageManager_q : MonoBehaviour
{
    public static StorageManager_q ins;

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

    /*   UDP Connect Method   */
    private IPEndPoint ipEndPoint;
    private UdpClient udpClient;
    private Thread receiveThread;

    /*   Parse Data Parameters   */
    public bool Data_Update = false;
    public bool Mahony_Update = false;

    // IMU data
    public int Number_Of_IMU = 6;
    public Quaternion Hand_q;

    private void Awake()
    {
        if (ins == null)
        {
            ins = this;
            GameObject.DontDestroyOnLoad(gameObject);
        }
        else if (ins != this)
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

            // Declare data rotation for correct hardware layout
            if (IMU_Index == 0)
                Storage.Hardware_Data_Correct_q[IMU_Index] = Quaternion.AngleAxis(-90, new Vector3(0, 0, 1));
            else
                Storage.Hardware_Data_Correct_q[IMU_Index] = Quaternion.AngleAxis(90, new Vector3(0, 0, 1));
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

        //else if (Storage.BLE && (Storage.BLE_Open == false))
        //{
        //    // Start scan surround device (to stabilize subscribe)
        //    BleApi.StartDeviceScan();
        //    BleApi.StopDeviceScan();

        //    // Start subscribe characteristic
        //    Debug.Log("Trying subscribe ...");
        //    BleApi.SubscribeCharacteristic(Device_ID, Service_ID, Device_Characteristic_ID, false);
        //    Storage.BLE_Open = true;
        //}

        //else if (Storage.Socket && (Storage.Socket_Open == false))
        //{
        //    ipEndPoint = new IPEndPoint(IPAddress.Parse("127.0.0.1"), 5000);
        //    udpClient = new UdpClient(ipEndPoint.Port);

        //    Storage.Socket_Open = true;
        //    Debug.Log("Socket Open");
        //}

        receiveThread = new Thread(ReceiveData);
        receiveThread.IsBackground = true;
        receiveThread.Start();
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    void ReceiveData()
    {
        while (true)
        {
            Storage.Frame_Counter++;

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
                        IMU_Q_Parse(buf, (int)((byte)255 - buf[0]));
                        Hand_q = Storage.MahonyFilters[0].q;
                        Mahony_Update = true;

                        Debug.LogFormat("qw: {0}, qx: {1}, qy: {2}, qz: {3}", Hand_q.w, Hand_q.x, Hand_q.y, Hand_q.z);
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
            //        IMU_Q_Parse(res.buf, 0);
            //        Mahony_Update = true;

            //        //Debug.LogFormat("ax: {0}, ay: {1}, az: {2}", Acc[0].x, Acc[0].y, Acc[0].z);
            //    }
            //    //Debug.LogFormat("BLE status : {0}", res);
            //}
            //else if (Storage.Socket && Storage.Socket_Open)
            //{
            //    //Debug.Log("Frame Counter: " + Storage.Frame_Counter);
            //    buf = udpClient.Receive(ref ipEndPoint);

            //    IMU_Q_Parse(buf, 0);
            //    Hand_q = Storage.MahonyFilters[0].q;
            //    Mahony_Update = true;

            //    Debug.LogFormat("qw: {0}, qx: {1}, qy: {2}, qz: {3}", Hand_q.w, Hand_q.x, Hand_q.y, Hand_q.z);
            //}
        }
    }

    // Parse data to IMU orientation
    public void IMU_Q_Parse(byte[] data_buf, int buf_shift)
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
            //Storage.Gesture_Reset_Correct_q[index] = Quaternion.Inverse(World_To_Unity_Rotation(Storage.MahonyFilters[index].q));
    }

    public Quaternion World_To_Unity_Rotation(Quaternion q)
    {
        return new Quaternion(q.x, q.z, q.y, q.w);
        //return new Quaternion(-q.x, -q.z, -q.y, q.w);
    }

    // Reset mahony update flag
    public void Reset_Mahony_Update_Flag()
    {
        Mahony_Update = false;
    }

    public void DisableThread()
    {
        udpClient.Close();
        receiveThread.Join();
        receiveThread.Abort();
        Debug.Log("Thread has closeed");
    }
}
