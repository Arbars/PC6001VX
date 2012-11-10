#ifndef AY8910_H_INCLUDED
#define AY8910_H_INCLUDED

#include "../typedef.h"


////////////////////////////////////////////////////////////////
// �N���X��`
////////////////////////////////////////////////////////////////
class cAY8910 {
protected:
	int RegisterLatch;
	BYTE Regs[16];
	int LastEnable;
	int UpdateStep;
	int PeriodA, PeriodB, PeriodC, PeriodN, PeriodE;
	int CountA, CountB, CountC, CountN, CountE;
	int VolA, VolB, VolC, VolE;
	BYTE EnvelopeA, EnvelopeB, EnvelopeC;
	BYTE OutputA, OutputB, OutputC, OutputN;
	int8_t CountEnv;
	BYTE Hold,Alternate,Attack,Holding;
	int RNG;
	int VolTable[32];
	
	void _AYWriteReg( BYTE, BYTE );		// ���W�X�^�����݃T�u
	void AYWriteReg( BYTE, BYTE );		// ���W�X�^������
	BYTE AYReadReg( BYTE );				// ���W�X�^�Ǎ���
	
	void SetClock( int, int );			// PSG�N���b�N�ݒ�
	void SetVolumeTable( int );			// ���ʐݒ�(�{�����[���e�[�u���ݒ�)
	
	// �|�[�g�A�N�Z�X�֐�
	virtual BYTE PortAread();
	virtual BYTE PortBread();
	virtual void PortAwrite( BYTE );
	virtual void PortBwrite( BYTE );
	
	virtual void PreWriteReg() = 0;		// ���W�X�^�ύX�O�̃X�g���[���X�V
	
public:
	cAY8910();							// �R���X�g���N�^
	virtual ~cAY8910();					// �f�X�g���N�^
	
	void Reset();						// ���Z�b�g
	
	int Update1Sample();				// �X�g���[��1Sample�X�V
};


#endif	// PSG_H_INCLUDED
