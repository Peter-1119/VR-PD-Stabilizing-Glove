using System;
using System.Collections.Generic;
using UnityEngine;

// BLE code reference: https://github.com/adabru/BleWinrtDll/tree/main

public class IMU_BLE : MonoBehaviour
{
    Coroutine BLE_Listen;

    public bool isScanningDevices = false;
    public bool isScanningServices = false;
    public bool isScanningCharacteristics = false;
    public bool isSubscribe = false;
    public bool SubcribeClose = false;

    public string BLE_device_name = "STM_Glove";
    private string Device_ID = "BluetoothLE#BluetoothLEb0:3c:dc:49:37:c9-00:80:E1:26:7A:BC";
    private string Service_ID = "{00001234-cc7a-482a-984a-7f2ed5b3e58f}";
    private string Device_Characteristic_ID = "{00005678-8e22-4541-9d4c-21edae82ed19}";

    Dictionary<string, Dictionary<string, string>> devices = new Dictionary<string, Dictionary<string, string>>();
    List<string> Services = new List<string>();
    List<string> Characteristics = new List<string>();

    // Start is called before the first frame update
    void Start()
    {
        //BLE_Listen = StartCoroutine();
        if (isScanningDevices)
        {
            Debug.Log("Start scan devices");
            BleApi.StartDeviceScan();
        }
        else if (isScanningServices)
        {
            Debug.LogFormat("Start scan ({0}) services", Device_ID);
            BleApi.ScanServices(Device_ID);
        }
        else if (isScanningCharacteristics)
        {

        }
        else if (isSubscribe)
        {
            Debug.Log("Start subscribe IMU data.");
            bool res = BleApi.SubscribeCharacteristic(Device_ID, Service_ID, Device_Characteristic_ID, false);
            Debug.Log("Subscribe status: " + res);
        }
    }

    // Update is called once per frame
    void Update()
    {
        BleApi.ScanStatus status;

        if (isScanningDevices)
        {
            BleApi.DeviceUpdate res = new BleApi.DeviceUpdate();
            do
            {
                status = BleApi.PollDevice(ref res, false);

                if (!devices.ContainsKey(res.id))
                {
                    devices[res.id] = new Dictionary<string, string>()
                    {
                        {"name", res.name },
                        {"isConnectable", res.isConnectable.ToString() }
                    };
                    if (devices[res.id]["name"] == BLE_device_name)
                    {
                        Debug.Log("Find device ID: " + res.id + ", name: " + devices[res.id]["name"]);

                        if (res.id.ToLower() == Device_ID.ToLower())
                        {
                            BleApi.StopDeviceScan();
                            isScanningDevices = false;
                            isScanningServices = true;
                            BleApi.ScanServices(Device_ID);
                            Debug.LogFormat("Start scan device ({0} - {1}) services", BLE_device_name, res.id);
                        }
                    }
                }
            } while (status == BleApi.ScanStatus.AVAILABLE);
        }

        if (isScanningServices)
        {
            BleApi.Service res = new BleApi.Service();
            do
            {
                status = BleApi.PollService(out res, false);
                if (status == BleApi.ScanStatus.AVAILABLE)
                {
                    //Debug.Log("Find service: " + res.uuid);

                    if (res.uuid == Service_ID)
                    {
                        Debug.Log("Find target service: " + res.uuid);
                        isScanningServices = false;
                        isScanningCharacteristics = true;
                        BleApi.ScanCharacteristics(Device_ID, Service_ID);
                    }
                }
                //else if (status == BleApi.ScanStatus.FINISHED)
                //{
                //    isScanningServices = false;
                //}
            } while (status == BleApi.ScanStatus.AVAILABLE);
        }

        if (isScanningCharacteristics)
        {
            BleApi.Characteristic res = new BleApi.Characteristic();
            do
            {
                status = BleApi.PollCharacteristic(out res, false);
                if (status == BleApi.ScanStatus.AVAILABLE)
                {
                    string name = res.userDescription != "no description available" ? res.userDescription : res.uuid;
                    //Debug.Log("Find Characteristic: " + name);
                    //Debug.Log("res.uuid: " + res.uuid);
                    //Debug.Log("Target  : " + Device_Characteristic_ID);

                    if (res.uuid == Device_Characteristic_ID)
                    {
                        Debug.Log("Find target characteristic: " + res.uuid);
                        isScanningCharacteristics = false;
                        isSubscribe = true;
                        BleApi.SubscribeCharacteristic(Device_ID, Service_ID, Device_Characteristic_ID, false);
                    }
                }
                //else if (status == BleApi.ScanStatus.FINISHED)
                //{
                //    isScanningCharacteristics = false;
                //}
            } while (status == BleApi.ScanStatus.AVAILABLE);
        }

        if (isSubscribe)
        {
            BleApi.BLEData res = new BleApi.BLEData();
            while (BleApi.PollData(out res, false))
            {
                Debug.Log(BitConverter.ToString(res.buf, 0, res.size));
            }
        }

        if (SubcribeClose)
        {
            BleApi.Quit();
            Debug.Log("Trying close subscribe");
        }
    }
}
