using System;
using UnityEngine;

public class IMU_BLE2 : MonoBehaviour
{
    public string Device_ID = "BluetoothLE#BluetoothLEb0:3c:dc:49:37:c9-00:80:e1:26:75:f8";
    public string Service_ID = "{00001234-cc7a-482a-984a-7f2ed5b3e58f}";
    public string Device_Characteristic_ID = "{00005678-8e22-4541-9d4c-21edae82ed19}";
    bool isSubscribe = false;

    // Start is called before the first frame update
    void Start()
    {
        BleApi.StartDeviceScan();
        BleApi.StopDeviceScan();
    }

    // Update is called once per frame
    void Update()
    {
        if (!isSubscribe)
        {
            Debug.Log("Trying subscribe ...");
            BleApi.SubscribeCharacteristic(Device_ID, Service_ID, Device_Characteristic_ID, false);
            isSubscribe = true;
        }

        BleApi.BLEData res = new BleApi.BLEData();
        while (BleApi.PollData(out res, false))
        {
            Debug.Log(BitConverter.ToString(res.buf, 0, res.size));
        }
    }
}
