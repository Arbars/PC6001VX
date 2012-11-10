#ifndef MC6847_H_INCLUDED
#define MC6847_H_INCLUDED

#include "../typedef.h"


////////////////////////////////////////////////////////////////
// �N���X��`
////////////////////////////////////////////////////////////////
class cMC6847core {
protected:
	BYTE COL_AN[5];			// �J���[�R�[�h(�A���t�@�j���[�����b�N)
	BYTE COL_SG[9];			// �J���[�R�[�h(�Z�~�O���t�B�b�N)
	BYTE COL_CG[10][4];		// �J���[�R�[�h(�J���[�O���t�B�b�N)
	BYTE COL_RG[2][2];		// �J���[�R�[�h(���m�N���O���t�B�b�N)
	
	bool CrtDisp;			// CRT�\����� true:�\�� false:��\��
	bool N60Win;			// �E�B���h�E�T�C�Y true:N60 false:N60m
	int Mode4Col;			// ���[�h4�J���[���[�h 0:���m 1:��/�� 2:��/�� 3:�s���N/�� 4:��/�s���N
	
	WORD VAddr;				// �����A�h���X�J�E���^
	WORD HAddr;				// �����A�h���X�J�E���^
	int RowCntA;			// �\�����C���J�E���^(�A���t�@�j���[�����b�N,�Z�~�O��)
	int RowCntG;			// �\�����C���J�E���^(�O���t�B�b�N)
	
	// �A�g���r���[�g�f�[�^
	BYTE AT_AG;
	BYTE AT_AS;
	BYTE AT_IE;
	BYTE AT_GM;
	BYTE AT_CSS;
	BYTE AT_INV;
	
	int GetW();									// �o�b�N�o�b�t�@���擾(�K��l)
	int GetH();									// �o�b�N�o�b�t�@�����擾(�K��l)
	
	virtual BYTE *GetBufAddr() = 0;				// �o�b�t�@�A�h���X�擾
	virtual int GetBufPitch() = 0;				// �o�b�t�@�s�b�`(1Line�o�C�g��)�擾
	virtual int GetBufHeight() = 0;				// �o�b�t�@�����擾
	
	virtual void LatchGMODE() = 0;				// �A�g���r���[�g�f�[�^���b�`(�O���t�B�b�N���[�h�̂�)
	virtual void LatchAttr() = 0;				// �A�g���r���[�g�f�[�^���b�`
	virtual BYTE GetAttr() = 0;					// �A�g���r���[�g�f�[�^�擾
	virtual BYTE GetVram() = 0;					// VRAM�f�[�^�擾
	virtual BYTE GetFont1( WORD ) = 0;			// Font1�f�[�^�擾
	
public:
	cMC6847core();								// �R���X�g���N�^
	virtual ~cMC6847core();						// �f�X�g���N�^
	
	virtual void UpdateBackBuf() = 0;			// �o�b�N�o�b�t�@�X�V
	
	void SetCrtDisp( bool );					// CRT�\����Ԑݒ�
	
	bool GetWinSize();							// �E�B���h�E�T�C�Y�擾
	
	int GetMode4Color();						// ���[�h4�J���[���[�h�擾
	void SetMode4Color( int );					// ���[�h4�J���[���[�h�ݒ�
};


class cMC6847_1 : public virtual cMC6847core {
protected:
	virtual BYTE GetFont0( WORD ) = 0;			// Font0(VDG Font)�f�[�^�擾
	BYTE GetBcol();								// �{�[�_�[�J���[�擾
	
	void Draw1line1( int );						// 1���C���`��(N60)
	
public:
	cMC6847_1();								// �R���X�g���N�^
	virtual ~cMC6847_1();						// �f�X�g���N�^
	
	void UpdateBackBuf();						// �o�b�N�o�b�t�@�X�V
};


class cMC6847_2 : public virtual cMC6847core {
protected:
	BYTE COL_AN2[16];		// �J���[�R�[�h(�A���t�@�j���[�����b�N 60m)
	BYTE COL_CG3[2][16];	// �J���[�R�[�h(�J���[�O���t�B�b�N 60m ���[�h3)
	BYTE COL_CG4[2][16];	// �J���[�R�[�h(�J���[�O���t�B�b�N 60m ���[�h4)
	
	bool Mk2CharMode;		// mk2 �\�����[�h true:�L�����N�^ false:�O���t�B�b�N
	bool Mk2GraphMode;		// mk2 �O���t�B�b�N�𑜓x true:160*200 false:320*200
	
	int Css1;				// �F�̑g�ݍ��킹
	int Css2;
	int Css3;
	
	virtual BYTE GetFont2( WORD ) = 0;			// Font2�f�[�^�擾
	BYTE GetBcol();								// �{�[�_�[�J���[�擾
	
	void Draw1line1( int );						// 1���C���`��(N60)
	void Draw1line2( int );						// 1���C���`��(N60m)
	
public:
	cMC6847_2();								// �R���X�g���N�^
	virtual ~cMC6847_2();						// �f�X�g���N�^
	
	void UpdateBackBuf();						// �o�b�N�o�b�t�@�X�V
};


#endif	// MC6847_H_INCLUDED
