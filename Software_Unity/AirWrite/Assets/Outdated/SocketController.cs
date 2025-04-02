using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net;
using System.Net.Sockets;
using System.Threading;

public class SocketController : MonoBehaviour
{
    private IPEndPoint ipEndPoint;
    private UdpClient udpClient;
    private Thread receiveThread;
    private byte[] receiveByte;
    private string receiveData = "";

    // Start is called before the first frame update
    void Start()
    {
        ipEndPoint = new IPEndPoint(IPAddress.Parse("127.0.0.1"), 5000);
        udpClient = new UdpClient(ipEndPoint.Port);

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
            receiveByte = udpClient.Receive(ref ipEndPoint);
            receiveData = System.Text.Encoding.UTF8.GetString(receiveByte);

            Debug.Log("接收到：" + receiveData);
        }
    }

    private void OnDisable()
    {
        udpClient.Close();
        receiveThread.Join();
        receiveThread.Abort();
    }

}
