using System;
using System.IO.Ports;
using System.Collections.Generic;
using UnityEngine;

public class IMU_SerialPort : MonoBehaviour
{
    // UART port name, baud rate and open status
    private SerialPort stream;
    public string port = "COM4";
    public int baudrate = 19200;
    bool isStreaming = false;

    // UART receive buffer and buffer shift (if receive buffer is not correct)
    byte[] buf = new byte[256];
    int buf_shift = 0;

    // IMU data
    public int Number_Of_IMU = 6;
    short ax, ay, az;
    short gx, gy, gz;
    //short mx, my, mz;
    public Vector3[] Acc = new Vector3[6];
    public Vector3[] Gyro = new Vector3[6];
    public Vector3[] Mag = new Vector3[6];

    // IMU data scale
    float Acc_scale = 1 / 16384.0f;
    float Gyro_scale = 1 / 32768.0f * 250.0f / 180.0f * 3.141592653589793f;
    //float Mag_scale = 0.15f;

    public void Open()
    {
        isStreaming = true;

        stream = new SerialPort(port, baudrate);
        stream.ReadTimeout = 1;
        stream.Open();
        Debug.Log("IMU SerialPort port was opened susccesfully!\n");
    }

    // Read serial port data
    public bool ReadSerialPort(int timeout = 1)
    {
        // Set reading timeout 
        stream.ReadTimeout = timeout;

        // Reset buffer shift and buffer
        buf_shift = 0;
        for (int i = 0; i < 180; i++)
            buf[i] = 0;

        try
        {
            // Read 180 bytes data from serial port
            stream.Read(buf, 0, 180);

            // if index 0 of the buffer is 0x0000 then drop
            if (buf[0] == 0)
                return false;

            // if data received is shifted then correct the data base on pivot
            buf_shift = (int)((byte)255 - buf[0]);
            return true;
        }

        // Serial port reading timeout
        catch (TimeoutException)
        {
            return false;
        }
    }

    public bool IMU_DataParse()
    {
        if (isStreaming == true)
        {
            if (ReadSerialPort(1) == true)
            {
                for (int index = 0; index < Number_Of_IMU; index++)
                {
                    // Parse accelerator data
                    ax = (short)(buf[20 - buf_shift + 20 * index] << 8 | buf[21 - buf_shift + 20 * index]);
                    ay = (short)(buf[22 - buf_shift + 20 * index] << 8 | buf[23 - buf_shift + 20 * index]);
                    az = (short)(buf[24 - buf_shift + 20 * index] << 8 | buf[25 - buf_shift + 20 * index]);
                    Acc[index] = new Vector3(ax * Acc_scale, ay * Acc_scale, az * Acc_scale);

                    // Parse Gyroscope data
                    gx = (short)(buf[26 - buf_shift + 20 * index] << 8 | buf[27 - buf_shift + 20 * index]);
                    gy = (short)(buf[28 - buf_shift + 20 * index] << 8 | buf[29 - buf_shift + 20 * index]);
                    gz = (short)(buf[30 - buf_shift + 20 * index] << 8 | buf[31 - buf_shift + 20 * index]);
                    Gyro[index] = new Vector3(gx * Gyro_scale, gy * Gyro_scale, gz * Gyro_scale);
                }

                return true;
            }
        }

        return false;
    }
}
