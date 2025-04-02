using UnityEngine;
using UnityEngine.SceneManagement;

public class Btn_Function : MonoBehaviour
{
    public void Switch_To_Main_Page()
    {
        SceneManager.LoadSceneAsync("MainPage");
        SM_Record_Btn_and_Event(0x00, 0x00);
    }

    public void Switch_To_Air_Write_Page()
    {
        SceneManager.LoadSceneAsync("AirWritePage");
        SM_Record_Btn_and_Event(0x02, 0x00);
    }

    public void Switch_To_Debug_Page()
    {
        SceneManager.LoadSceneAsync("DebugPage");
        SM_Record_Btn_and_Event(0x01, 0x00);
    }

    public void Switch_To_Static_Gesture_Match_Page()
    {
        SceneManager.LoadSceneAsync("StaticGestureMatchPage");
        SM_Record_Btn_and_Event(0x03, 0x00);
    }

    public void Switch_To_Dynamic_Gesture_Match_Page()
    {
        SceneManager.LoadSceneAsync("DynamicGestureMatchPage");
        SM_Record_Btn_and_Event(0x05, 0x00);
    }

    public void Clear_Line_Renderer()
    {
        GameObject obj = GameObject.Find("EventSystem");
        AirWrite Air_Write = obj.GetComponent<AirWrite>();

        Air_Write.Clear_Line_Renderer();
        SM_Record_Btn_and_Event(0x02, 0x01);
    }

    public void Static_Gesture_Match_Start()
    {
        GameObject obj = GameObject.Find("EventSystem");
        StaticGestureMatch Static_Gesture_Match = obj.GetComponent<StaticGestureMatch>();

        Static_Gesture_Match.StartGame();
        SM_Record_Btn_and_Event(0x04, 0x00);
    }

    public void q_reset()
    {
        GameObject obj = GameObject.Find("StorageManager");
        StorageManager SM = obj.GetComponent<StorageManager>();

        SM.Correct_Gestures_q();
        SM_Record_Btn_and_Event(0x07, 0x00);
    }

    public void Record_Gesture()
    {
        GameObject obj = GameObject.Find("EventSystem");
        DynamicGestureMatch Dynamic_Gesture_Match = obj.GetComponent<DynamicGestureMatch>();

        Dynamic_Gesture_Match.StartRecord();
    }

    public void Play_Gesture()
    {
        GameObject obj = GameObject.Find("EventSystem");
        DynamicGestureMatch Dynamic_Gesture_Match = obj.GetComponent<DynamicGestureMatch>();

        Dynamic_Gesture_Match.StartPlay();
        SM_Record_Btn_and_Event(0x06, 0x00);
    }

    public void CloseThread()
    {
        GameObject obj = GameObject.Find("StorageManager");
        StorageManager_q SM = obj.GetComponent<StorageManager_q>();

        SM.DisableThread();
    }

    public void GlobalRecord()
    {
        Storage.Global_Record = !Storage.Global_Record;
    }

    public void SM_Record_Btn_and_Event(byte Btn_Index, byte Event_Index)
    {
        GameObject obj = GameObject.Find("StorageManager");
        StorageManager SM = obj.GetComponent<StorageManager>();

        SM.Record_Btn_and_Event(Btn_Index, Event_Index);
    }
}
