using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using UnityEngine.UI;

public class UDPClient : MonoBehaviour
{
    private Thread _recvThread;
    private IPEndPoint ipEndPoint;
    private UdpClient udpClient;

    public Button Btn;

    // Receive Buffer
    byte[] buf = new byte[1024];

    // Start is called before the first frame update
    void Start()
    {
        Btn.onClick.AddListener(TestFun);

        ipEndPoint = new IPEndPoint(IPAddress.Parse("127.0.0.1"), 5000);
        udpClient = new UdpClient(5000);
        //udpClient.Connect("127.0.0.1", 5000);

        _recvThread = new Thread(ReceiveData);
        _recvThread.IsBackground = true;
        _recvThread.Start();

        Debug.Log("Socket Open");
    }

    // Update is called once per frame
    void Update()
    {

    }

    void TestFun()
    {
        Debug.Log("Button is clicking.");

        udpClient.Close();
        _recvThread.Join();
        _recvThread.Abort();

        Debug.Log("Thread has closeed");
    }

    void ReceiveData()
    {
        while (true)
        {
            buf = udpClient.Receive(ref ipEndPoint);
            string receivedMessage = System.Text.Encoding.ASCII.GetString(buf);
            Debug.Log("data: " + receivedMessage);
        }
    }
}
