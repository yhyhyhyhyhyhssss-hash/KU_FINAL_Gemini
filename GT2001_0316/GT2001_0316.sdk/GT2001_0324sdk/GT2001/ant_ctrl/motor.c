
#include "all_include.h"

//=============================================================

void Motor_AZStop(void)
{
	MotorSpd.AZ=0.0F;
	MotorSpd.SetAZ=0.0F;
	MotorSetPos.EnAZ=FALSE;
	MotorSpd.EnAZ=FALSE;
}

void Motor_ELStop(void)
{
	MotorSpd.EL=0.0F;
	MotorSetPos.EnEL=FALSE;
	MotorSpd.EnEL=FALSE;
}

void Motor_POLStop(void)
{
	MotorSpd.POL=0.0F;
	MotorSetPos.EnPOL=FALSE;
	MotorSpd.EnPOL=FALSE;
}

void Motor_Stop(void)
{
	// TrModule_SwitchStatuesUpdate(SWITCH_OFF,SWITCH_OFF);   // 关闭TR模块开关
}

void Motor_SetPosition(float tposaz,float tposel,float tposrpol,bool PlsSet)
{
	// az
	//tposaz = 360.0f-tposaz;
	if (tposaz<0.0F)		tposaz = tposaz + 360.0F;
	else if (tposaz>360.0F)	tposaz = tposaz - 360.0F; 
	MotorSetPos.AZ	= tposaz;
	
	tposaz = tposaz + CodeZero.ZeroAZ;
	if (tposaz<0.0F)		tposaz = tposaz + 360.0F;
	else if (tposaz>360.0F)	tposaz = tposaz - 360.0F; 
	MotorSetPos.FinalAZ = tposaz;
	MotorTrackDif.AZ_Ok = TRUE;
	
	// el
	if (tposel>MaxPosEL) 		tposel = MaxPosEL;
	else if (tposel<MinPosEL) 	tposel = MinPosEL;
	MotorSetPos.EL = tposel;

//	 tposel = tposel + DetPosEL + CodeZero.ZeroEL; 			//抛物面天线
	tposel = 90.0f - (tposel + DetPosEL + CodeZero.ZeroEL); 		//相控阵天线，离轴角
	if (tposel>MaxPosEL) 		tposel = MaxPosEL;
	else if (tposel<MinPosEL) 	tposel = MinPosEL;
	MotorSetPos.FinalEL = tposel;
	MotorTrackDif.EL_Ok = TRUE;
	
	// pol
	if (tposrpol>MaxPosPOL) 		tposrpol = MaxPosPOL;
	else if (tposrpol<MinPosPOL) 	tposrpol = MinPosPOL;
	
//	sinAz = sin(tposaz);
//	CosAz = cos(tposaz);
//	CosEl = cos(tposel);
//	detPOl = atan(-sinAz/(CosEl*CosAz));
//	detPOl = atan(sinAz/(CosEl*CosAz));
//	tposrpol = tposrpol + CodeZero.ZeroPOL + detPOl;
//	tposrpol = tposrpol + CodeZero.ZeroPOL + tposaz;
//	tposrpol = detPOl + CodeZero.ZeroPOL;
//	tposrpol = detPOl + TargetData.POL + CodeZero.ZeroPOL;
//	tposrpol = tposaz + TargetData.POL + CodeZero.ZeroPOL;
	tposrpol = tposaz + tposrpol + CodeZero.ZeroPOL;
//	tposrpol = CodeZero.ZeroPOL + tposaz;
	if (tposrpol<0.0f) 	tposrpol += 180.0f;		// 转换为0~180
	if (tposrpol>360.0F) tposrpol -= 360.0f;
	if (tposrpol>180.0F) tposrpol -= 180.0f;
	MotorSetPos.POL = tposrpol;

	MotorSetPos.FinalPOL = tposrpol;
	MotorTrackDif.POL_Ok = TRUE;


	TrModulePara.AZ_BeamPoint = MotorSetPos.FinalAZ;
	TrModulePara.EL_BeamPoint = MotorSetPos.FinalEL;
	TrModulePara.POL_BeamPoint= MotorSetPos.FinalPOL;
	//TrModule_SendCmd_DeamPoint();
}

void Motor_SetSpeed(float tspdaz,float tspdel,float tspdpol)
{
	/*
	//az
	if (tspdaz<MinSpdAZ)	tspdaz=MinSpdAZ;
	else if (tspdaz>MaxSpdAZ)	tspdaz=MaxSpdAZ; 

	MotorSpd.AZ=tspdaz;//NewSpeedAz;
	
	//el
	if (tspdel>MaxSpdEL) 	tspdel=MaxSpdEL;
	else if (tspdel<MinSpdEL) 	tspdel=MinSpdEL;
	
	MotorSpd.EL=tspdel;
	
	if (!MotorSpd.EnEL)
	{
		Motor_ELStop();
	}
	else if (((PosCode.PosFinalEL>MaxPosEL)&&(MotorSpd.EL>0.0F))||((PosCode.PosFinalEL<MinPosEL)&&(MotorSpd.EL<0.0F)))
	{
		Motor_ELStop();
	}
	else
	{
		//Motor_SetSpdEL(MotorSpd.SetEL);
	}
	*/
}

//==========================================================================

/*!
    @brief  跟踪控制函数
    @param  Saz		当前方位角误差
    @param  Sel		当前俯仰角误差
    @param  UsTar	当前参数类型 
					0:目标参数AZ + 目标参数EL
					1:目标参数AZ + 理论参数EL
					2:理论参数AZ + 理论参数EL
    @retval 
	@attention
*/
void FollowControl(float Saz,float Sel,uint8_t UsTar)
{		
	static float Taz=0.0F,Tel=0.0F;

//	InsData.RealHead=InsData.fHead+InsData.SeekDetHead;
//	if (InsData.RealHead>360.0F)	InsData.RealHead=InsData.RealHead-360.0F;
//	else if (InsData.RealHead<0.0F)	InsData.RealHead=InsData.RealHead+360.0F;
	
	InsData.ftarPitch = InsData.fixPitch + InsData.fDetPitch;
	InsData.ftarRoll  = InsData.fixRoll  + InsData.fDetRoll;
	InsData.ftarHead  = InsData.RealHead + InsData.fDetHead;
//	InsData.ftarHead=InsData.fixHead+InsData.fDetHead;

	if (InsData.ftarHead>360.0F)	InsData.ftarHead = InsData.ftarHead - 360.0F;
	else if (InsData.ftarHead<0.0F)	InsData.ftarHead = InsData.ftarHead + 360.0F;

	switch(UsTar)
	{
		case 0:	
		/* 姿态位置环,目标参数AZ + 目标参数EL */
			Taz = TargetData.AZ + Saz;
			Tel = TargetData.EL + Sel;
			if (Taz>360.0F)		Taz = Taz-360.0F;
			else if (Taz<0.0F)	Taz = Taz+360.0F;

			if (Tel>90.0F)		Tel = 90.0F;
			else if (Tel<0.0F)	Tel = 0.0F;
			TargetData.sAZ = Taz;
			TargetData.sEL = Tel;
		
			Ins2ANTProBDD(TargetData,InsData);	// 惯导信息+天线位置解算
			Motor_SetPosition(Ins2Ant.tNTarAZ,Ins2Ant.tNTarEL,Ins2Ant.tNTarPOL,FALSE);
			break;
		case 1:
		/* 姿态位置环,目标参数AZ，理论角度EL */
			Taz = InsData.ftarHead + Saz;
			if (Taz>360.0F)		Taz=Taz-360.0F;
			else if (Taz<0.0F)	Taz=Taz+360.0F;
			InsData.ftarHead = Taz;
			TargetData.sAZ = TargetData.AZ;
			TargetData.sEL = TargetData.EL;

			Ins2ANTProBDD(TargetData,InsData);	// 惯导信息+天线位置解算

			Tel = Ins2Ant.tNTarEL + Sel;
			if (Tel > MaxPosEL)			Tel = MaxPosEL;
			else if (Tel < MinPosEL)	Tel = MinPosEL;
		
			Motor_SetPosition(Ins2Ant.tNTarAZ,Tel,Ins2Ant.tNTarPOL,FALSE);
			break;
		default: 
		/* 姿态位置环,理论角度AZ，理论角度EL */
			TargetData.sAZ=TargetData.AZ;
			TargetData.sEL=TargetData.EL;

			Ins2ANTProBDD(TargetData,InsData);

			Taz = Ins2Ant.tNTarAZ + Saz;
			if (Taz>360.0F)		Taz=Taz-360.0F;
			else if (Taz<0.0F)	Taz=Taz+360.0F;
		
			Tel = Ins2Ant.tNTarEL + Sel;
			if (Tel>MaxPosEL)		Tel = MaxPosEL;
			else if (Tel<MinPosEL)	Tel = MinPosEL;
		
			Motor_SetPosition(Taz,Tel,Ins2Ant.tNTarPOL,FALSE);
			break;
	}
}

//==========================================================================




