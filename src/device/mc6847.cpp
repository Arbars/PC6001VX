#include <stdlib.h>

#include "../log.h"
#include "mc6847.h"


#define	P6WIDTH		(256)	/* �����L���\������(N60)     */
#define	P6HEIGHT	(192)	/* �����L���\�����C��(N60)   */
#define	P2WIDTH		(320)	/* �����L���\������(N60m)    */
#define	P2HEIGHT	(200)	/* �����L���\�����C��(N60m)  */

// �o�b�N�o�b�t�@�T�C�Y
#define	WBBUF		(376-16)	/* �K��4�̔{�� */
#define	HBBUF		(242-12)

// �{�[�_�[�T�C�Y
#define	LB60		((int)((WBBUF-P6WIDTH)/2))
#define	RB60		(WBBUF-P6WIDTH-LB60)
#define	TB60		((int)((HBBUF-P6HEIGHT)/2))
#define	BB60		(HBBUF-P6HEIGHT-TB60)
#define	LB62		((int)((WBBUF-P2WIDTH)/2))
#define	RB62		(WBBUF-P2WIDTH-LB62)
#define	TB62		((int)((HBBUF-P2HEIGHT)/2))
#define	BB62		(HBBUF-P2HEIGHT-TB62)

// �A�g���r���[�g
#define ATTR_AG		0x80
#define ATTR_AS		0x40
#define ATTR_INTEXT	0x20
#define ATTR_GM0	0x10
#define ATTR_GM1	0x08
#define ATTR_GM2	0x04
#define ATTR_CSS	0x02
#define ATTR_INV	0x01

#define ANMODE		( ( AT_AS << 1 ) | AT_IE )
#define AM_AN0		0		/* �A���t�@�j���[�����b�N(�����t�H���g) */
#define AM_AN1		1		/* �A���t�@�j���[�����b�N(�O���t�H���g) */
#define AM_SG4		2		/* �Z�~�O���t�B�b�N4 */
#define AM_SG6		3		/* �Z�~�O���t�B�b�N6 */

#define GM_CG1		0		/*  64x 64 �J���[	*/
#define GM_CG2		1		/* 128x 64 �J���[	*/
#define GM_CG3		2		/* 128x 96 �J���[	*/
#define GM_CG6		3		/* 128x192 �J���[	*/
#define GM_RG1		4		/* 128x 64 ���m�N��	*/
#define GM_RG2		5		/* 128x 96 ���m�N��	*/
#define GM_RG3		6		/* 128x192 ���m�N��	*/
#define GM_RG6		7		/* 256x192 ���m�N��	*/




////////////////////////////////////////////////////////////////
// �R���X�g���N�^
////////////////////////////////////////////////////////////////
cMC6847core::cMC6847core( void ) :
	CrtDisp(true), N60Win(true),
	Mode4Col(0), VAddr(0), HAddr(0), RowCntA(0), RowCntG(0),
	AT_AG(0), AT_AS(0), AT_IE(0), AT_GM(0), AT_CSS(0), AT_INV(0) {}

cMC6847_1::cMC6847_1( void ){}

cMC6847_2::cMC6847_2( void ) : 
	Mk2CharMode(true), Mk2GraphMode(false), Css1(1), Css2(1), Css3(1) {}


////////////////////////////////////////////////////////////////
// �f�X�g���N�^
////////////////////////////////////////////////////////////////
cMC6847core::~cMC6847core( void ){}

cMC6847_1::~cMC6847_1( void ){}

cMC6847_2::~cMC6847_2( void ){}


////////////////////////////////////////////////////////////////
// �o�b�N�o�b�t�@���擾(�K��l)
////////////////////////////////////////////////////////////////
int cMC6847core::GetW( void )
{
	return WBBUF;
}


////////////////////////////////////////////////////////////////
// �o�b�N�o�b�t�@�����擾(�K��l)
////////////////////////////////////////////////////////////////
int cMC6847core::GetH( void )
{
	return HBBUF;
}


////////////////////////////////////////////////////////////////
// CRT�\����Ԑݒ�
////////////////////////////////////////////////////////////////
void cMC6847core::SetCrtDisp( bool st )
{
	CrtDisp = st;
}


////////////////////////////////////////////////////////////////
// �E�B���h�E�T�C�Y�擾
////////////////////////////////////////////////////////////////
bool cMC6847core::GetWinSize( void )
{
	return N60Win;
}


////////////////////////////////////////////////////////////////
// ���[�h4�J���[���[�h�擾
////////////////////////////////////////////////////////////////
int cMC6847core::GetMode4Color( void )
{
	return Mode4Col;
}


////////////////////////////////////////////////////////////////
// ���[�h4�J���[���[�h�ݒ�
////////////////////////////////////////////////////////////////
void cMC6847core::SetMode4Color( int col )
{
	Mode4Col = col;
}


////////////////////////////////////////////////////////////////
// �o�b�N�o�b�t�@�X�V
//
// ����:	�Ȃ�
// �Ԓl:	�Ȃ�
////////////////////////////////////////////////////////////////
void cMC6847_1::UpdateBackBuf( void )
{
	PRINTD( GRP_LOG, "[VDG][60][UpdateBackBuf]\n" );
	
	BYTE *doff = GetBufAddr();
	
	VAddr = HAddr = RowCntA = RowCntG = 0;
	
	// �㑤�{�[�_�`��
	for( int i=0; i<TB60*GetBufPitch(); i++ ) *(doff++) = GetBcol();
	
	// �\���G���A�`��
	for( int i=0; i<P6HEIGHT; i++ ) Draw1line1( i );
	
	// �����{�[�_�`��
	doff = GetBufAddr() + ( TB60 + P6HEIGHT ) * GetBufPitch();
	for( int i=0; i<BB60*GetBufPitch(); i++ ) *(doff++) = GetBcol();
}

void cMC6847_2::UpdateBackBuf( void )
{
	PRINTD( GRP_LOG, "[VDG][62][UpdateBackBuf]\n" );
	
	BYTE *doff = GetBufAddr();
	
	VAddr = HAddr = RowCntA = 0;
	
	if( N60Win ){	// N60
		// �㑤�{�[�_�`��
		for( int i=0; i<TB60*GetBufPitch(); i++ ) *(doff++) = GetBcol();
		
		// �\���G���A�`��
		for( int i=0; i<P6HEIGHT; i++ ) Draw1line1( i );
		
		// �����{�[�_�`��
		doff = GetBufAddr() + ( TB60 + P6HEIGHT ) * GetBufPitch();
		for( int i=0; i<BB60*GetBufPitch(); i++ ) *(doff++) = GetBcol();
	}else{			// N60m
		// �㑤�{�[�_�`��
		for( int i=0; i<TB62*GetBufPitch(); i++ ) *(doff++) = GetBcol();
		
		// �\���G���A�`��
		for( int i=0; i<P2HEIGHT; i++ ) Draw1line2( i );
		
		// �����{�[�_�`��
		doff = GetBufAddr() + ( TB62 + P2HEIGHT ) * GetBufPitch();
		for( int i=0; i<BB62*GetBufPitch(); i++ ) *(doff++) = GetBcol();
	}
}


////////////////////////////////////////////////////////////////
// 1���C���`��(N60)
//
// ����:	line	�`�悷�郉�C���ԍ�(0-191)
// �Ԓl:	�Ȃ�
////////////////////////////////////////////////////////////////
void cMC6847_1::Draw1line1( int line )
{
	BYTE data, fdat=0, fg=0, bg=0;
	BYTE LAT_AG=0, LAT_GM=0;
	
	// �o�b�N�o�b�t�@�A�h���X���߂�
	BYTE *doff = GetBufAddr() + ( TB60 + line ) * GetBufPitch();
	
	// �����{�[�_�[�`��
	for( int i=0; i<LB60; i++ ) *(doff++) = GetBcol();
	
	// �\���G���A�`��
	for( int x=0; x<( P6WIDTH / 8 ); x++ ){
		if( CrtDisp ){
			LatchAttr();
			data = GetVram();
			HAddr++;
			LAT_AG |= AT_AG;				// �Ƃ肠����
			if( x == 2 ) LAT_GM = AT_GM;	// �Ƃ肠����
			
			// 1byte��̃O���t�B�b�N���[�h�擾
			if( LAT_AG ) LatchGMODE();
				// ����@��VDG(���Ӊ�H?)�ɂ̓o�O������炵��
				// �A�g���r���[�g�A�h���X�ɑ΂��J���[�Z�b�g�͐������ǂ߂邪
				// ���[�h����̎��ɂ�1byte��̃A�h���X����ǂ�ł��܂��悤��
				// �E�[�̏ꍇ�͂��̃��C���̐擪(���[)�̃f�[�^��ǂނ炵��
				// mk2,66�ł͂ǂ�����������ǂ߂�
		}else{
			data = AT_AG ? rand() : 0;	// �z���g�H ��łǂ��ɂ�����
		}
		
		if( AT_AG ){	// �O���t�B�b�N
			switch( AT_GM ){
			case GM_CG1:	//  64x 64 �J���[   (CG1)
				for( int i=3; i>=0; i-- ){
					fdat = COL_CG[AT_CSS][(data>>(i*2))&3];
					for( int j=0; j<4; j++ ) *(doff++) = fdat;
				}
				x++;
				break;
				
			case GM_CG2:	// 128x 64 �J���[   (CG2)
			case GM_CG3:	// 128x 96 �J���[   (CG3)
			case GM_CG6:	// 128x192 �J���[   (CG6)
				for( int i=6; i>=0; i-=2 ){
					fdat = COL_CG[AT_CSS][(data>>i)&3];
					*(doff++) = fdat;
					*(doff++) = fdat;
				}
				break;
				
			case GM_RG1:	// 128x 64 ���m�N�� (RG1)
			case GM_RG2:	// 128x 96 ���m�N�� (RG2)
			case GM_RG3:	// 128x192 ���m�N�� (RG3)
				for( int i=7; i>=0; i-- ){
					fdat = COL_RG[AT_CSS][(data>>i)&1];
					*(doff++) = fdat;
					*(doff++) = fdat;
				}
				x++;
				break;
				
			case GM_RG6:	// 256x192 ���m�N�� (RG6)
				if( Mode4Col ){	// 128x192 �J���[(�ɂ���)
					int CsC = AT_CSS + Mode4Col*2;
					for( int i=6; i>=0; i-=2 ){
						fdat = COL_CG[CsC][(data>>i)&3];
						*(doff++) = fdat;
						*(doff++) = fdat;
					}
				}else{			// 256x192 ���m�N��
					for( int i=7; i>=0; i-- )
						*(doff++) = COL_RG[AT_CSS][(data>>i)&1];
				}
			}
			
		}else{			// �A���t�@�j���[�����b�N
			switch( ANMODE ){
			case AM_AN0:		// �A���t�@�j���[�����b�N(�����t�H���g)
				fg   = COL_AN[(AT_CSS<<1) |   AT_INV];
				bg   = COL_AN[(AT_CSS<<1) | (~AT_INV&1)];
				fdat = GetFont0( (data&0x3f)*16 + RowCntA );
				break;
				
			case AM_AN1:		// �A���t�@�j���[�����b�N(�O���t�H���g)
				fg   = COL_AN[(AT_CSS<<1) |   AT_INV];
				bg   = COL_AN[(AT_CSS<<1) | (~AT_INV&1)];
				fdat = GetFont1( data*16 + RowCntA );
				break;
				
			case AM_SG4:		// �Z�~�O���t�B�b�N4
				fg   = COL_SG[(data>>4)&7];
				bg   = COL_SG[8];
				fdat = (data<<(4+(RowCntA/6)*2)&0x80) | (data<<(1+(RowCntA/6)*2)&0x08);
				fdat |= fdat>>1 | fdat>>2 | fdat>>3;
				break;
				
			case AM_SG6:		// �Z�~�O���t�B�b�N6
				fg   = COL_SG[(AT_CSS<<2) | ((data>>6)&3)];
				bg   = COL_SG[8];
				fdat = (data<<(2+(RowCntA/4)*2)&0x80) | (((data<<(3+(RowCntA/4)*2))>>4)&0x08);
				fdat |= fdat>>1 | fdat>>2 | fdat>>3;
			}
			for( int i=7; i>=0; i-- )
				*(doff++) = (fdat>>i)&1 ? fg : bg;
			
			
		};
	}
	
	
	// �������A�h���X�I�t�Z�b�g���߂�
	//   �ǂ����e���C��3byte�ڂ̕\�����[�h�Ō��܂��Ă���炵��
	//   ���������ꃉ�C���ɈقȂ郂�[�h�����݂���Ƃ��͋������ς��
	if( LAT_AG ){	// �O���t�B�b�N
		switch( LAT_GM ){
		case GM_CG1:	//  64x 64 �J���[   (CG1)
		case GM_CG2:	// 128x 64 �J���[   (CG2)
		case GM_RG1:	// 128x 64 ���m�N�� (RG1)
			if( RowCntG++ == 2 ){
				VAddr++;
				RowCntG = 0;
			}
			break;
		case GM_CG3:	// 128x 96 �J���[   (CG3)
		case GM_RG2:	// 128x 96 ���m�N�� (RG2)
			if( RowCntG++ == 1 ){
				VAddr++;
				RowCntG = 0;
			}
			break;
		case GM_CG6:	// 128x192 �J���[   (CG6)
		case GM_RG3:	// 128x192 ���m�N�� (RG3)
		case GM_RG6:	// 256x192 ���m�N�� (RG6)
			VAddr++;
		}
		RowCntA = 0;
	}else{			// �A���t�@�j���[�����b�N
		if( RowCntA++ == 11 ){
			VAddr++;
			RowCntA = 0;
		}
		RowCntG = 0;
	}
	HAddr = 0;
	
	// �E���{�[�_�[�`��
	doff = GetBufAddr() + ( TB60 + line ) * GetBufPitch() + LB60 + P6WIDTH;
	for( int i=0; i<RB60; i++ ) *(doff++) = GetBcol();
}

void cMC6847_2::Draw1line1( int line )
{
	BYTE data, fdat=0, fg=0, bg=0;
	BYTE LAT_AG=0;
	
	// �o�b�N�o�b�t�@�A�h���X���߂�
	BYTE *doff = GetBufAddr() + ( TB60 + line ) * GetBufPitch();
	
	// �����{�[�_�[�`��
	for( int i=0; i<LB60; i++ ) *(doff++) = GetBcol();
	
	// �\���G���A�`��
	for( int x=0; x<( P6WIDTH / 8 ); x++ ){
		if( CrtDisp ){
			LatchAttr();
			data = GetVram();
			HAddr++;
			LAT_AG |= AT_AG;				// �Ƃ肠����
		}else{
			data = 0;	// �z���g�HRGB�o�͂ƃr�f�I�o�͂ňقȂ�͂�
		}
		
		if( AT_AG ){	// �O���t�B�b�N
			switch( AT_GM ){
			case GM_CG1:	//  64x 64 �J���[   (CG1)(����)
			case GM_CG2:	// 128x 64 �J���[   (CG2)(����)
			case GM_CG3:	// 128x 96 �J���[   (CG3)(����)
			case GM_CG6:	// 128x192 �J���[   (CG6)
				for( int i=6; i>=0; i-=2 ){
					fdat = COL_CG[AT_CSS][(data>>i)&3];
					*(doff++) = fdat;
					*(doff++) = fdat;
				}
				break;
				
			case GM_RG1:	// 128x 64 ���m�N�� (RG1)(����)
			case GM_RG2:	// 128x 96 ���m�N�� (RG2)(����)
			case GM_RG3:	// 128x192 ���m�N�� (RG3)(����)
			case GM_RG6:	// 256x192 ���m�N�� (RG6)
				if( Mode4Col ){	// 128x192 �J���[(�ɂ���)
					int CsC = AT_CSS + Mode4Col*2;
					for( int i=6; i>=0; i-=2 ){
						fdat = COL_CG[CsC][(data>>i)&3];
						*(doff++) = fdat;
						*(doff++) = fdat;
					}
				}else{			// 256x192 ���m�N��
					for( int i=7; i>=0; i-- )
						*(doff++) = COL_RG[AT_CSS][(data>>i)&1];
				}
			}
		}else{			// �A���t�@�j���[�����b�N
			switch( ANMODE ){
			case AM_AN0:		// �A���t�@�j���[�����b�N(�����t�H���g)(����)
			case AM_AN1:		// �A���t�@�j���[�����b�N(�O���t�H���g)
				fg   = COL_AN[(AT_CSS<<1) |   AT_INV];
				bg   = COL_AN[(AT_CSS<<1) | (~AT_INV&1)];
				fdat = GetFont1( data*16 + RowCntA );
				break;
				
			case AM_SG4:		// �Z�~�O���t�B�b�N4(����)
			case AM_SG6:		// �Z�~�O���t�B�b�N6
				fg   = COL_SG[(AT_CSS<<2) | ((data>>6)&3)];
				bg   = COL_SG[8];
				fdat = GetFont1( 0x1000 + (data&0x3f)*16 + RowCntA );	// �Z�~�O���t�H���g�� 1000H-13FFH
			}
			for( int i=7; i>=0; i-- )
				*(doff++) = (fdat>>i)&1 ? fg : bg;
		};
	}
	
	
	// �������A�h���X�I�t�Z�b�g���߂�
	//   �ǂ����e���C��3byte�ڂ̕\�����[�h�Ō��܂��Ă���炵��
	//   ���������ꃉ�C���ɈقȂ郂�[�h�����݂���Ƃ��͋������ς��
	//   mk,66���������ǂ����͕s��������������ƃV���v�����Ǝv����
	if( LAT_AG ){	// �O���t�B�b�N
		VAddr++;
		RowCntA = 0;
	}else{			// �A���t�@�j���[�����b�N
		if( RowCntA++ == 11 ){
			VAddr++;
			RowCntA = 0;
		}
	}
	HAddr = 0;
	
	// �E���{�[�_�[�`��
	doff = GetBufAddr() + ( TB60 + line ) * GetBufPitch() + LB60 + P6WIDTH;
	for( int i=0; i<RB60; i++ ) *(doff++) = GetBcol();
}


////////////////////////////////////////////////////////////////
// 1���C���`��(N60m)
//
// ����:	line	�`�悷�郉�C���ԍ�(0-199)
// �Ԓl:	�Ȃ�
////////////////////////////////////////////////////////////////
void cMC6847_2::Draw1line2( int line )
{
	BYTE attr, data;
	
	// �o�b�N�o�b�t�@�A�h���X���߂�
	BYTE *doff = GetBufAddr() + ( TB62 + line ) * GetBufPitch();
	
	// �����{�[�_�[�`��
	for( int i=0; i<LB62; i++ ) *(doff++) = GetBcol();
	
	// �\���G���A�`��
	for( int x=0; x<( P2WIDTH / 8 ); x++){
		if( CrtDisp ){
			attr = GetAttr();
			data = GetVram();
			HAddr++;
		}else{
			attr = 0;	// �z���g�H
			data = 0;	// �z���g�H
		}
		
		if( Mk2CharMode ){	// �L�����N�^���[�h
			BYTE fg   = COL_AN2[attr&0x0f];
			BYTE bg   = COL_AN2[((attr>>4)&0x07)|((Css2&1)<<3)];
			BYTE fdat = GetFont2( (data+((attr&0x80)?256:0))*16 + RowCntA );
			for( int i=7; i>=0; i-- )
				*(doff++) = (fdat>>i)&1 ? fg : bg;
		}else{				// �O���t�B�b�N���[�h
			if( Mk2GraphMode ){	// �O���t�B�b�N ���[�h3
				for( int i=6; i>=0; i-=2 ){
					BYTE fdat = COL_CG3[Css3][(((data<<2)>>i)&0x0c)|((attr>>i)&3)];
					*(doff++) = fdat;
					*(doff++) = fdat;
				}
			}else{				// �O���t�B�b�N ���[�h4
				BYTE c = (Css2<<3)|(Css1<<2);
				for( int i=7; i>=0; i-- )
					*(doff++) = COL_CG4[Css3][c|(((data<<1)>>i)&2)|((attr>>i)&1)];
			}
		}
	}
	
	
	// �������A�h���X�I�t�Z�b�g���߂�
	if( Mk2CharMode ){	// �A���t�@�j���[�����b�N
		if( RowCntA++ == 9 ){
			VAddr++;
			RowCntA = 0;
		}
	}else{				// �O���t�B�b�N
		VAddr++;
		RowCntA = 0;
	}
	HAddr = 0;
	
	// �E���{�[�_�[�`��
	doff = GetBufAddr() + ( TB62 + line ) * GetBufPitch() + LB62 + P2WIDTH;
	for( int i=0; i<RB62; i++ ) *(doff++) = GetBcol();
}


////////////////////////////////////////////////////////////////
// �{�[�_�[�J���[�擾
//
// ����:	�Ȃ�
// �Ԓl:	BYTE	�{�[�_�[�J���[�R�[�h
////////////////////////////////////////////////////////////////
BYTE cMC6847_1::GetBcol( void )
{
	BYTE bcol;
	
	if( AT_AG ){	// �O���t�B�b�N���[�h
		if( AT_GM & 4 ) bcol = COL_RG[AT_CSS][1];	// ���m�N��
		else			bcol = COL_CG[AT_CSS][0];	// �J���[
	}else{			// �A���t�@�j���[�����b�N���[�h
		if( AT_AS ) 	bcol = COL_SG[8];			// �Z�~�O���t�B�b�N
		else			bcol = COL_AN[4];			// �A���t�@�j���[�����b�N
	}
	
	return bcol;
}

BYTE cMC6847_2::GetBcol( void )
{
	BYTE bcol;
	
	// ���ۂ͑S����������
	if( N60Win )               bcol = COL_AN[4];		// N60
	else{												// N60m
		if( Mk2CharMode )      bcol = COL_AN2[0];		// �L�����N�^���[�h
		else{
			if( Mk2GraphMode ) bcol = COL_CG3[0][0];	// �O���t�B�b�N ���[�h3
			else			   bcol = COL_CG4[0][0];	// �O���t�B�b�N ���[�h4
		}
	}
	
	return bcol;
}


