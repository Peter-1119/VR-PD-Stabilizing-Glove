using System.IO.Ports;
using UnityEngine;

public static class Storage
{
    public static int Frame_Counter = 0;
    
    public static bool Serial_Wire = false;
    public static bool Serial_Wire_Open = false;
    //public static bool BLE = false;
    //public static bool BLE_Open = false;
    //public static bool Socket = false;
    //public static bool Socket_Open = false;
    public static bool Global_Record = false;
    public static bool Global_Play = false;

    public static SerialPort stream;

    // Mahony Filter
    public static Mahony[] MahonyFilters = new Mahony[6];
    //public List<Mahony> MahonyFilters = new List<Mahony>();

    // Correct q
    public static Quaternion[] Hardware_Data_Correct_q = new Quaternion[6];
    public static Quaternion[] Gesture_Reset_Correct_q = new Quaternion[6];
}
