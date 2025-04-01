using System.Collections;
using UnityEngine;

public class Mahony : MonoBehaviour
{
    public float delta_t = 0.02f;
    private const float Kp = 1.0f;
    private const float Ki = 0.1f;

    public Quaternion q = new Quaternion(0, 0, 0, 1);

    public Vector3 eInt = new Vector3(0, 0, 0);

    public Vector3 gravity = new Vector3(0, 0, 1);
    
    public Vector3 qpq(Quaternion q, Vector3 p)
    {
        float new_x = (1 - 2 * q.y * q.y - 2 * q.z * q.z) * p.x + (2 * q.x * q.y - 2 * q.w * q.z) * p.y + (2 * q.x * q.z + 2 * q.w * q.y) * p.z;
        float new_y = (2 * q.x * q.y + 2 * q.w * q.z) * p.x + (1 - 2 * q.x * q.x - 2 * q.z * q.z) * p.y + (2 * q.y * q.z - 2 * q.w * q.x) * p.z;
        float new_z = (2 * q.x * q.z - 2 * q.w * q.y) * p.x + (2 * q.y * q.z + 2 * q.w * q.x) * p.y + (1 - 2 * q.x * q.x - 2 * q.y * q.y) * p.z;

        return new Vector3(new_x, new_y, new_z);
    }

    public Quaternion q_prod(Quaternion q1, Quaternion q2)
    {
        float new_qw = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
        float new_qx = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
        float new_qy = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
        float new_qz = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;

        return new Quaternion(new_qx, new_qy, new_qz, new_qw);
    }

    public Quaternion q_update(Quaternion q, Vector3 p)
    {
        float new_qw = q.w + 0.5f * (-q.x * p.x - q.y * p.y - q.z * p.z) * delta_t;
        float new_qx = q.x + 0.5f * (q.w * p.x + q.y * p.z - q.z * p.y) * delta_t;
        float new_qy = q.y + 0.5f * (q.w * p.y - q.x * p.z + q.z * p.x) * delta_t;
        float new_qz = q.z + 0.5f * (q.w * p.z + q.x * p.y - q.y * p.x) * delta_t;

        return new Quaternion(new_qx, new_qy, new_qz, new_qw);
    }

    public Quaternion update_q(Vector3 Acc, Vector3 Gyro)
    {
        Vector3 Correct_Gyro = new Vector3(0, 0, 0);

        if (Vector3.Distance(new Vector3(0, 0, 0), Acc) != 0)
        {
            Vector3 Est_Acc = qpq(Quaternion.Inverse(q), gravity);
            Vector3 e = Vector3.Cross(Est_Acc, Vector3.Normalize(Acc));

            eInt = eInt + Ki * e * delta_t;
            Correct_Gyro = Gyro - Kp * e - eInt;
        }
        else
        {
            Correct_Gyro = Gyro;
        }

        q = Quaternion.Normalize(q_update(q, Correct_Gyro));

        return q;
    }
}
