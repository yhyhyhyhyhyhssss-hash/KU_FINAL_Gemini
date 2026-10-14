/*
 * atti.c
 *
 *  Created on: 2025年3月23日
 *      Author: Ewen
 */

#include "atti.h"
#include "sensor.h"
#include "math.h"

// 定义常量
#define SAMPLE_RATE     50.0f  // 采样频率 (Hz)
#define BETA            0.1f    // 加速度计校正系数

// 定义PI值
#define PI 3.14159265358979323846f

// 四元数结构体
typedef struct {
	float q0, q1, q2, q3;
} Quaternion;

// 将角度转换为弧度
float to_radians(float degrees) {
    return degrees * PI / 180.0f;
}

// 将弧度转换为角度
float to_degrees(float radians) {
    return radians * 180.0f / PI;
}

// 从欧拉角计算四元数
Quaternion euler_to_quaternion(float roll, float pitch, float yaw) {
    // 将角度转换为弧度
    roll = to_radians(roll);
    pitch = to_radians(pitch);
    yaw = to_radians(yaw);

    // 计算半角
    float cy = cos(yaw * 0.5f);
    float sy = sin(yaw * 0.5f);
    float cp = cos(pitch * 0.5f);
    float sp = sin(pitch * 0.5f);
    float cr = cos(roll * 0.5f);
    float sr = sin(roll * 0.5f);

    // 计算四元数
    Quaternion q;
    q.q0 = cr * cp * cy + sr * sp * sy;
    q.q1 = sr * cp * cy - cr * sp * sy;
    q.q2 = cr * sp * cy + sr * cp * sy;
    q.q3 = cr * cp * sy - sr * sp * cy;

    return q;
}


// 从四元数计算欧拉角 俯仰角：-90~+90 横滚角：-180~+180 航向角：-180~+180
void quaternion_to_euler(Quaternion q, float *roll, float *pitch, float *yaw) {
    *roll = atan2(2.0f * (q.q0 * q.q1 + q.q2 * q.q3), 1.0f - 2.0f * (q.q1 * q.q1 + q.q2 * q.q2));
    *pitch = asin(2.0f * (q.q0 * q.q2 - q.q3 * q.q1));
    *yaw = atan2(2.0f * (q.q0 * q.q3 + q.q1 * q.q2), 1.0f - 2.0f * (q.q2 * q.q2 + q.q3 * q.q3));

    // 将弧度转换为角度
    *roll = to_degrees(*roll);
    *pitch = to_degrees(*pitch);
    *yaw = to_degrees(*yaw);
}


// 计算俯仰角（pitch）和横滚角（roll）
void calculate_angles(float ax, float ay, float az, float *pitch, float *roll) {
	// 归一化加速度值
	float norm = sqrt(ax * ax + ay * ay + az * az);
	ax /= norm;
	ay /= norm;
	az /= norm;

    // 计算俯仰角（pitch）
    *pitch = atan2(ay, sqrt(ax * ax + az * az));

    // 计算横滚角（roll）
    *roll = atan2(-ax, az);

    // 将弧度转换为角度
    *pitch = to_degrees(*pitch);
    *roll = to_degrees(*roll);
}


// 初始化四元数
void quaternion_init(Quaternion *q) {
    q->q0 = 1.0f;
    q->q1 = 0.0f;
    q->q2 = 0.0f;
    q->q3 = 0.0f;
}

// 四元数归一化
void quaternion_normalize(Quaternion *q) {
    float norm = sqrtf(q->q0 * q->q0 + q->q1 * q->q1 + q->q2 * q->q2 + q->q3 * q->q3);
    q->q0 /= norm;
    q->q1 /= norm;
    q->q2 /= norm;
    q->q3 /= norm;
}

// 四元数更新（陀螺仪积分）
void quaternion_update(Quaternion *q, float gx, float gy, float gz, float dt) {

	// 将角度转换为弧度
	gx = to_radians(gx);
	gy = to_radians(gy);
	gz = to_radians(gz);

    // 角速度转换为弧度
    gx *= dt;
    gy *= dt;
    gz *= dt;

    // 四元数微分方程
    Quaternion q_dot = {
        -0.5f * (gx * q->q1 + gy * q->q2 + gz * q->q3),
         0.5f * (gx * q->q0 + gy * q->q3 - gz * q->q2),
         0.5f * (-gx * q->q3 + gy * q->q0 + gz * q->q1),
         0.5f * (gx * q->q2 - gy * q->q1 + gz * q->q0)
    };

    // 更新四元数
    q->q0 += q_dot.q0;
    q->q1 += q_dot.q1;
    q->q2 += q_dot.q2;
    q->q3 += q_dot.q3;

    // 归一化四元数
    quaternion_normalize(q);
}

// 加速度计校正
void quaternion_correct(Quaternion *q, float ax, float ay, float az) {
    // 加速度计数据归一化
    float norm = sqrtf(ax * ax + ay * ay + az * az);
    ax /= norm;
    ay /= norm;
    az /= norm;

    // 计算误差
    float vx = 2.0f * (q->q2 * q->q3 - q->q0 * q->q1);
    float vy = 2.0f * (q->q0 * q->q2 + q->q1 * q->q3);
    float vz = q->q0 * q->q0 - q->q1 * q->q1 - q->q2 * q->q2 + q->q3 * q->q3;

    float error_x = ay * vz - az * vy;
    float error_y = az * vx - ax * vz;
    float error_z = ax * vy - ay * vx;

    // 校正四元数
    q->q0 -= BETA * error_x;
    q->q1 -= BETA * error_y;
    q->q2 -= BETA * error_z;

    // 归一化四元数
    quaternion_normalize(q);
}

Quaternion q;

void atti_init(void)
{
    // 初始化四元数
    quaternion_init(&q);
}

void atti_test1(void) {
    // 示例输入：欧拉角（单位：度）
    float roll = 89.0f;   // 横滚角
    float pitch = -89.0f;  // 俯仰角
    float yaw = 179.0f;    // 方位角

    // 计算四元数
    Quaternion q = euler_to_quaternion(roll, pitch, yaw);

    // 输出四元数
    gs_print("Quaternion: w = %f, x = %f, y = %f, z = %f\r\n", q.q0, q.q1, q.q2, q.q3);

    // 计算欧拉角
    quaternion_to_euler(q, &roll, &pitch, &yaw);

    // 输出结果
    gs_print("Roll: %f, Pitch: %f, Yaw: %f\r\n", roll, pitch, yaw);

}


void atti_test2(void) {

    float roll = 0;   // 横滚角
    float pitch = 0;  // 俯仰角

    calculate_angles(-acceleration_mg[1] / 1000, acceleration_mg[0] / 1000, acceleration_mg[2] / 1000, &pitch, &roll);

    // 输出结果
    gs_print("Pitch: %f, Roll: %f \r\n", pitch, roll);

}


void atti_test3(void) {

    // 更新姿态
    float dt = 1.0f / SAMPLE_RATE;
    quaternion_update(&q, angular_rate_mdps[0] / 1000, angular_rate_mdps[1] / 1000, angular_rate_mdps[2] / 1000, dt);  // 陀螺仪积分
    quaternion_correct(&q, acceleration_mg[0] / 1000, acceleration_mg[1] / 1000, acceleration_mg[2] / 1000);     // 加速度计校正

    // 计算欧拉角
    float pitch, roll, yaw;
    quaternion_to_euler(q, &pitch, &roll, &yaw);

    // 输出结果
    gs_print("Pitch: %f, Roll: %f, Yaw: %f\r\n", pitch, roll, yaw);
}





