using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;

public class SearchFiles : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        DirectoryInfo info = new DirectoryInfo("Gesture_Record");
        FileInfo[] fileInfo = info.GetFiles();
        foreach (FileInfo f in fileInfo)
        {
            Debug.Log("filename: " + Path.GetFileName(f.ToString()));
        }
    }

    // Update is called once per frame
    void Update()
    {

    }

    // Reference Website: https://discussions.unity.com/t/get-list-of-all-files-in-a-directory/3356/4
    bool CheckFile(string filename)
    {
        string[] files = Directory.GetFiles("Gesture_Record");
        foreach (string f in files)
        {
            if (f == filename)
                return true;
        }
        return false;
    }

    // Reference website: https://forum.unity.com/threads/help-making-a-text-file-when-in-game.848125/
    void WriteFile(string filename, string content)
    {
        if (!File.Exists(filename))
            File.WriteAllText(filename, content);
        else
            File.AppendAllText(filename, content);
    }

    // Reference website: https://support.unity.com/hc/en-us/articles/115000341143-How-do-I-read-and-write-data-from-a-text-file
    void WriteFile2(string filename, bool append = true)
    {
        StreamWriter writer = new StreamWriter(filename, append);
        writer.Write("Hey, Peter\n");
        writer.Write("Nice to meet you\n");
        writer.Close();
    }

    // Reference website: https://www.mongodb.com/developer/products/realm/saving-data-in-unity3d-using-binary-reader-writer/
    void WriteFile3(string filename)
    {
        FileStream fileStream = File.Open(filename, FileMode.Create);
        BinaryWriter binaryWriter = new BinaryWriter(fileStream);
        binaryWriter.Write(23.0f);
        binaryWriter.Write(53.2f);
        fileStream.Close();
    }

    void ReadFile(string filename)
    {
        StreamReader reader = new StreamReader("test3.txt");

        while (!reader.EndOfStream)
        {
            string inp_ln = reader.ReadLine();
            Debug.Log(inp_ln);
        }
    }

    // Reference website: https://support.unity.com/hc/en-us/articles/115000341143-How-do-I-read-and-write-data-from-a-text-file
    void ReadFile2(string filename)
    {
        StreamReader reader = new StreamReader(filename);
        Debug.Log(reader.ReadToEnd());
    }

    // Reference website: https://www.mongodb.com/developer/products/realm/saving-data-in-unity3d-using-binary-reader-writer/
    // Reference Website: https://stackoverflow.com/questions/10942848/c-sharp-checking-for-binary-reader-end-of-file
    void ReadFile3(string filename)
    {
        float value = 0;

        FileStream fileStream = File.Open(filename, FileMode.Open);
        BinaryReader binaryReader = new BinaryReader(fileStream);

        while (binaryReader.BaseStream.Position != binaryReader.BaseStream.Length)
        {
            value = binaryReader.ReadSingle();
            Debug.Log(value);
        }

        fileStream.Close();
    }
}
