using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;


// Reference Website: https://zhuanlan.zhihu.com/p/376599128
public class SocketClient
{
    private Socket _clientSocket;
    private Thread _recvThread;
    private string _ServerIP;
    private int _ServerPort;

    public SocketClient(string ServerIP, int ServerPort)
    {
        this._ServerIP = ServerIP;
        this._ServerPort = ServerPort;
        Connect();
    }

    public void Connect()
    {
        try
        {
            _clientSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            _clientSocket.Connect(new IPEndPoint(IPAddress.Parse(_ServerIP), _ServerPort));
            StartRecv();
        }
        catch
        {
            throw;
        }
    }

    private void StartRecv()
    {
        _recvThread = new Thread(OnRecv);
        _recvThread.IsBackground = true;
        Debug.Log("Start listening");
        _recvThread.Start();
    }

    private void OnRecv()
    {
        while (true)
        {
            byte[] buff = new byte[1024];
            int length = _clientSocket.Receive(buff);
            if (length != 0)
                Debug.Log(Encoding.Default.GetString(buff));
        }
    }

    public void Close()
    {
        _recvThread.Abort();
        _clientSocket.Shutdown(SocketShutdown.Both);
        _clientSocket.Close();
    }
}
