/*
 * ant_ctrl.c
 *
 *  Created on: 2025楠烇拷3閺堬拷8閺冿拷
 *      Author: 86158
 */

/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
	*RTSOTM BJBOE LEO GD V2.1
	*MCU:GD32F450ZGTx
  ******************************************************************************
  */
#include "all_include.h"
#include "Openamip.h"
#include "delay.h"
#include "ant_dbg.h"
#include <math.h>
#include "../src/bsp.h"
//=============手动模式========================
uint8_t  auto_mode=0;//0:手动指向 1：自动指向 (默认手动指向模式)
extern uint32_t Adcl_enable;
//==============================================================================

uint8_t  InitRec=0;
uint8_t TrSendSwtich = 0;
float LostRecoverTime=0.0F;

uint16_t antRfCount=0;
uint16_t tCount=0;
uint16_t antRTempCount=0;
uint16_t antTTempCount=0;
bool antRpadgetflagA=FALSE;
bool antRpadgetflagB=FALSE;
bool antTpadgetflagA=FALSE;
bool antTpadgetflagB=FALSE;
bool antRChnFlag=FALSE;
bool antTChnFlag=FALSE;

void RunStatusCheck(void)
{
	//fault check==========================
	if (ItemFault.ModemRecvTimeOut < 200)		ItemFault.Modem = FALSE;
	else 										ItemFault.Modem = TRUE;

	if (ItemFault.InsRecvTimeOut < 200)			ItemFault.Ins = FALSE;
	else										ItemFault.Ins = TRUE;

	if (ItemFault.TRModuleRecvTimeOut < 200) 	ItemFault.TrModule = FALSE;
	else 										ItemFault.TrModule = TRUE;

	if (ItemFault.BeaconRecvTimeOut < 200) 		ItemFault.Beacon = FALSE;
	else 										ItemFault.Beacon = TRUE;

	if(ItemFault.Ins)	ItemFault.Gps = TRUE;
	else				ItemFault.Gps = FALSE;

	ItemFault.Statues = 0;
	ItemFault.Statues = (ItemFault.Ins      << 7)
					  | (ItemFault.Modem    << 6)
					  | (ItemFault.TrModule << 5)
					  | (ItemFault.Gps		<< 4)
					  | (ItemFault.Beacon 	<< 3);
}



//Timer10ms_TIM_Period================================================
void Timer10ms_TIM_Period(void)
{
	ItemFault.ModemRecvTimeOut++;
	ItemFault.InsRecvTimeOut++;
	ItemFault.TRModuleRecvTimeOut++;
	ItemFault.BeaconRecvTimeOut++;
	ItemFault.BeaconSendTimeOut++;
}


//SOTM_CONTROL================================================================
void SOTM_CONTROL(void)
{
	//static uint16_t CtrlRunCount=0;
	static first_start_counter = 0;
	static uint16_t clcTarCount=0;
	static uint16_t insUpdateCount=0;
	RunStatusCheck();		//
	P2P_TargeClc();			//

//	if ((GpsSet)||(!InsData.Gps_Ok_Flag))
//	{
//		if (GpsSet)
//		{
//			InsData.Gps_Ok		= TRUE;
//			InsData.Gps_Ok_Flag = TRUE;
//		}
//		InsData.fGpsJD	= YZ_GPS.JD ;
//		InsData.fGpsWD	= YZ_GPS.WD ;
//		InsData.fGpsGDm	= YZ_GPS.GD ;
//		InsData.fGpsGDkm= InsData.fGpsGDm/1000.0;
//	}


	clcTarCount++;
	if (clcTarCount >= 200)  //
	{
		clcTarCount = 0;
		if (((1 == YZ_GPS.Save)||('Y' == YZ_GPS.Save))&&(1 == gnss_rt_t.gnss_valid))
		{
			if ((fabs(YZ_GPS.JD-InsData.fGpsJD) > 0.2F)||(fabs(YZ_GPS.WD-InsData.fGpsWD) > 0.2F))
			{
				YZ_GPS.JD = gnss_rt_t.longitude;
				YZ_GPS.WD = gnss_rt_t.latitude;
				YZ_GPS.GD = gnss_rt_t.altitude;
				Flash_Write_GPS(YZ_GPS);
			}
		}
	}

	insUpdateCount++;
	if (insUpdateCount > 49)
	{	// INS 250Hz output , 4ms x 50 = 200ms 5Hz
		insUpdateCount = 0;
		if(Satellite.STrackMode == Track_Follow)
		{
			if(1== AdvconfigInfo.FixInsEnable)
			{
				//UpdateHead(1,1,1,50);
				UpdateHead(DetHeadFlag,DetHeadEnFlag,DetHeadFirstFlag,InsData.RealHead);
			}
		}
	}
	TimAntCtrl_OK = true;	//

	// 与 KU_V4 保持一致：闭库 SeekSatellite/FastSeek/Follow2Axis 自主维护信标与锁星状态，外部不强行干预
	// Get_Agc_Values();

	// 状态转移调试日志
	static RunMode_EnumType s_last_run_mode = ModePointTrack;
	if (SysInfos.RunMode != s_last_run_mode)
	{
		gs_print("SOTM: 模式切换 [%d -> %d]\r\n", (int)s_last_run_mode, (int)SysInfos.RunMode);
		s_last_run_mode = SysInfos.RunMode;
	}

	//=============================
	switch(SysInfos.RunMode) {
		case ModeFirstStart:
			/* 默认手动对星模式下直接进入手动指向，无需死等10秒进入自动寻星 */
			if(auto_mode == 0)
			{
				StartRun = FALSE;
				SysInfos.RunMode = ModePointTrack;
				FollowControl(0.0F,0.0F,1);
				break;
			}
			first_start_counter ++;
			if(first_start_counter > (200*10))
			{
				/* 自动模式下等待GPS和惯导信号OK */
				StartRun = FALSE;
				if(Satellite.STrackMode == Track_Follow)    SysInfos.RunMode = ModeSeekSat;
				else                                        SysInfos.RunMode = ModePointTrack;
			}
			else {
				/* 保持 AZ,EL 零位 */
				FollowControl(0.0F,0.0F,1);
			}
			break;
		case ModeSeekSat:
			auto_mode = 1;
			Adcl_enable = 1;
			Satellite.STrackMode = Track_Follow;
			SeekSatellite(BeaconData.Agc,SeekSteps);
			break;
		case ModeSeekSatFast:
			auto_mode = 1;
			Adcl_enable = 1;
			Satellite.STrackMode = Track_Follow;
			FastSeek(BeaconData.Agc,SeekSteps);
			break;
		case ModeFollow:
			if(Satellite.STrackMode==Track_Follow)
			{
				Follow2Axis(FALSE,BeaconData.Agc);
				g_acu_data.s_modem_trans = 1;
				LostRecoverTime = 0.0F; /* 稳定跟踪时重置快速搜星超时计时器 */
			}
			else
			{
				SysInfos.RunMode = ModePointTrack;
			}
			break;
		case ModePointTrack:
            //
			TrackCircleStep = 0;
			FollowControl(0.0F,0.0F,1);
			break;
		case ModeLostRecover:
//			SeekSatellite(BeaconData.Agc,SeekSteps);
			if(Satellite.STrackMode == Track_Follow) FastSeek(BeaconData.Agc,SeekSteps);
			else                                     SysInfos.RunMode = ModePointTrack;

			LostRecoverTime = LostRecoverTime + AdvconfigInfo.AntCtrlPeriodTime;
			if (LostRecoverTime > 90.0F) // 快速搜星超时(90秒未能重捕)，转入大范围全空域自动搜星
			{
				SysInfos.RunMode = ModeSeekSat; // 修复：直接转入 ModeSeekSat 重新大范围搜星，避免死锁
				SeekSteps = 0;
				SeekLock = FALSE;
				LostRecoverTime  = 0.0F;
				gs_print("SOTM: 快速搜星超时(90s)，切回大范围自动寻星 (ModeSeekSat)\r\n");
			}
			break;
		case ModeTest:
			auto_mode = 0;
			_monitorEnable = FALSE;
			break;
		case ModeMoveSpeed:

			// Motor_SetSpeed(MotorSpd.SetAZ,MotorSpd.SetEL,MotorSpd.SetPOL);
			break;
		case ModeMovePos:

			// Motor_SetPosition(MotorSetPos.SetAZ,MotorSetPos.SetEL,MotorSetPos.SetPOL,FALSE);
			break;
		case ModeFixInsZero:
			break;
		case ModeIdle:
			if(auto_mode == 0)
			{
				SysInfos.RunMode = ModePointTrack;
				break;
			}
			if((InsData.Imu_Ok)||(InsData.Ins_Ok))
			{
				if(Satellite.STrackMode == Track_Follow)    SysInfos.RunMode = ModeSeekSat;
				else                                        SysInfos.RunMode = ModePointTrack;
			}
			break;

		case ModeEmergency:
			break;
		case ModeStop:
			//Motor_Stop();
			break;
		case ModeLock:
			//Motor_Stop();
			break;
		default:
			//Motor_Stop();
			break;
	}
	/* ACU RX DATA PROCESS =========================================================*/
	if (g_AcuUartData.recv_complete)
	{
		// debug acu
		g_AcuUartData.recv_complete = FALSE;
		CmdPro_Process(g_AcuRxData);
	}
	if(_monitorEnable) {
		if(ModemReportFlag) {	//
			ModemReportFlag=FALSE;
			//Modem_Status_Report();
		}
		if(ReportInfoFlag) {	//
			ReportInfoFlag=FALSE;
			acu_system_data_send();
		}
		if(ReportStatusFlag) {	//
			ReportStatusFlag=FALSE;
			acu_system_status_send();
		}
	}

}

void ant_ctrl_data_init(void)
{

	/* SYSTEM INITIALIZE +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
	Data_Init();
	Data_Load();
	mDelaymS(50);

	/* START INFORMATION +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
	ant_print("System Initialize OK !\r\n");

	ant_print("Project \t:RTSOTM_SCM320MA \r\n");

	ant_print("Hardware Verion\t:V%.2f\r\n",SysInfos.HardVersion);

	ant_print("Software Verion\t:V%.2f\r\n",SysInfos.SoftVersion);

	ant_print("Antenna Control System Start!\r\n");

}




