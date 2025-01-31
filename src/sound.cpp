/////////////////////////////////////////////////////////////////////////////
//  P C 6 0 0 1 V
//  Copyright 1999,2022 Yumitaro
/////////////////////////////////////////////////////////////////////////////
#include <mutex>

#include "pc6001v.h"

#include "error.h"
#include "log.h"
#include "osd.h"
#include "sound.h"


// バッファサイズの倍率
#define	MULTI		(2)
//   内部的にはサイズがMULTI倍されるが対外的には等倍のように振舞う
//   読み書きは全領域に対して行なわれるがサイズ取得は等倍
//   つまりオーバーフローを防止しつつレスポンスを保てる...はず


/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
cRing::cRing( void ) : Size(0) {}


/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
cRing::~cRing( void ){}


/////////////////////////////////////////////////////////////////////////////
// バッファ初期化
//
// 引数:	size		サンプル数
// 返値:	bool		true:成功 false:失敗
/////////////////////////////////////////////////////////////////////////////
bool cRing::InitBuffer( int size )
{
	PRINTD( SND_LOG, "[cRing][Init] Size:%d\n", size );
	
	std::deque<int32_t>().swap( Buffer );
	Size = size;
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// 読込み
//
// 引数:	なし
// 返値:	int		データ
/////////////////////////////////////////////////////////////////////////////
int cRing::Get( void )
{
	std::lock_guard<cMutex> lock( Mutex );
	
	int ret = 0;
	if( !Buffer.empty() ){
		ret = Buffer.front();
		Buffer.pop_front();
	}
	
	return ret;
}


/////////////////////////////////////////////////////////////////////////////
// 書込み
//
// 引数:	data		データ
// 返値:	なし
/////////////////////////////////////////////////////////////////////////////
void cRing::Put( int data )
{
	std::lock_guard<cMutex> lock( Mutex );
	
	if( (int)Buffer.size() < (Size * (MULTI+1)) ){	// +1は保険
		Buffer.emplace_back( data );
	}
}


/////////////////////////////////////////////////////////////////////////////
// 未読データ数取得
//
// 引数:	なし
// 返値:	int			未読サンプル数
/////////////////////////////////////////////////////////////////////////////
int cRing::ReadySize( void ) const
{
	std::lock_guard<cMutex> lock( Mutex );
	
	return Buffer.size();
}


/////////////////////////////////////////////////////////////////////////////
// バッファサイズ取得
//
// 引数:	なし
// 返値:	int			全バッファサンプル数
/////////////////////////////////////////////////////////////////////////////
int cRing::GetSize( void ) const
{
	std::lock_guard<cMutex> lock( Mutex );
	
	return Size;
}




/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
SndDev::SndDev( void ) : SampleRate(DEFAULT_SAMPLERATE),
							Volume(0), LPF_Mem(0), LPF_fc(0) {}


/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
SndDev::~SndDev( void ){}


/////////////////////////////////////////////////////////////////////////////
// ローパスフィルタ カットオフ周波数設定
//
// 引数:	fc		カットオフ周波数
// 返値:	なし
/////////////////////////////////////////////////////////////////////////////
void SndDev::SetLPF( int fc )
{
	LPF_fc = fc;
}


/////////////////////////////////////////////////////////////////////////////
// ローパスフィルタ
//
// [参考] 初歩のディジタルフィルタ
// http://kmkz.jp/mtm/mag/lab/digitalfilter.htm
//
// 引数:	src		元データ
// 返値:	int		フィルタ適用後のデータ
/////////////////////////////////////////////////////////////////////////////
int SndDev::LPF( int src )
{
	int lpf_k = LPF_fc ? (int)((2.0 * M_PI * (double)LPF_fc * (double)0x8000) / (double)SampleRate) : 0x8000;
	
	LPF_Mem += ( ( src - LPF_Mem ) * lpf_k ) / 0x8000;
	return LPF_Mem;
}


/////////////////////////////////////////////////////////////////////////////
// 読込み(オーバーライド)
//
// 引数:	なし
// 返値:	int		データ
/////////////////////////////////////////////////////////////////////////////
int SndDev::Get( void )
{
	return LPF( (this->cRing::Get() * Volume) / 100 );
}


/////////////////////////////////////////////////////////////////////////////
// サンプリングレート設定
//
// 引数:	rate	サンプリングレート
//			size	バッファサイズ
// 返値:	bool	true:成功 false:失敗
/////////////////////////////////////////////////////////////////////////////
bool SndDev::SetSampleRate( int rate, int size )
{
	SampleRate = rate;
	
	if( !this->cRing::InitBuffer( size ) ){
		return false;
	}
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// 音量設定
//
// 引数:	vol		音量(0-100)
// 返値:	なし
/////////////////////////////////////////////////////////////////////////////
void SndDev::SetVolume( int vol )
{
	Volume = min( max( vol, 0 ), 100 );
}


/////////////////////////////////////////////////////////////////////////////
// ストリーム更新
//
// 引数:	samples	更新するサンプル数
// 返値:	int		更新したサンプル数
/////////////////////////////////////////////////////////////////////////////
int SndDev::SoundUpdate( int samples )
{
	return 0;
}




/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
SND6::SND6( void ) : Volume(DEFAULT_MASTERVOL), SampleRate(DEFAULT_SAMPLERATE), BSize(DEFAULT_SOUNDBUF),
						CbFunc(nullptr), CbData(nullptr) {}


/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
SND6::~SND6( void )
{
	Pause();
	// オーディオデバイスを閉じる
	OSD_CloseAudio();
	
	sdev.clear();
}


/////////////////////////////////////////////////////////////////////////////
// 初期化
//
// 引数:	cbdata		コールバック関数に渡すデータ
//			callback	コールバック関数へのポインタ
//			rate		サンプリングレート
//			size		バッファサイズ(倍率)
// 返値:	bool		true:成功 false:失敗
/////////////////////////////////////////////////////////////////////////////
bool SND6::Init( void* cbdata, void (*callback)(void*, BYTE*, int ), int rate, int size )
{
	PRINTD( SND_LOG, "[SND6][Init]\n" );
	
	sdev.clear();
	
	// バッファサイズ(サンプル数,2のべき乗)
	int samples = (rate / 11025) << (size - 1 + 8);
	
	// バッファ初期化
	if( !this->cRing::InitBuffer( samples ) ){
		return false;
	}
	
	// オーディオデバイスを開く
	if( !OSD_OpenAudio( cbdata, callback, rate, samples ) ){
		return false;
	}
	
	CbData     = cbdata;
	CbFunc     = callback;
	SampleRate = rate;
	BSize      = size;
	
	PRINTD( SND_LOG, " SampleRate : %d\n", SampleRate );
	PRINTD( SND_LOG, " BufferSize : %d\n", samples );
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// ストリーム接続
//
// 引数:	buf			バッファポインタ
// 返値:	bool		true:成功 false:失敗
/////////////////////////////////////////////////////////////////////////////
bool SND6::ConnectStream( const std::shared_ptr<SndDev>& sd )
{
	PRINTD( SND_LOG, "[SND6][ConnectStream]\n" );
	
	if( !sd->SetSampleRate( SampleRate, this->cRing::GetSize() ) ){
		return false;
	}
	sdev.emplace_back( sd );
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// 再生
//
// 引数:	なし
// 返値:	なし
/////////////////////////////////////////////////////////////////////////////
void SND6::Play( void )
{
	PRINTD( SND_LOG, "[SND6][Play]\n" );
	
	OSD_StartAudio();
}


/////////////////////////////////////////////////////////////////////////////
// 停止
//
// 引数:	なし
// 返値:	なし
/////////////////////////////////////////////////////////////////////////////
void SND6::Pause( void )
{
	PRINTD( SND_LOG, "[SND6][Pause]\n" );
	
	OSD_StopAudio();
}


/////////////////////////////////////////////////////////////////////////////
// サンプリングレート設定
//
// 引数:	rate		サンプリングレート
//			size		バッファサイズ(倍率)
// 返値:	bool		true:成功 false:失敗
/////////////////////////////////////////////////////////////////////////////
bool SND6::SetSampleRate( int rate, int size )
{
	PRINTD( SND_LOG, "[SND6][SetSampleRate]\n" );
	
	// 現在の再生状態を保存した上で一旦オーディオデバイスを閉じる
	bool pflag = OSD_AudioPlaying();
	OSD_CloseAudio();
	
	// バッファサイズ(サンプル数,2のべき乗)
	int samples = (rate / 11025) << ((size ? size : BSize) - 1 + 8);
	
	// バッファ初期化
	if( !this->cRing::InitBuffer( samples ) ){
		return false;
	}
	
	// デバイス毎にサンプリングレートを設定
	for( auto &p : sdev ){
		if( !p->SetSampleRate( rate, samples ) ){
			return false;
		}
	}
	
	// オーディオデバイスを開く
	if( !OSD_OpenAudio( CbData, CbFunc, rate, samples ) ){
		return false;
	}
	
	SampleRate = rate;
	BSize      = size ? size : BSize;
	
	// 再生中だったなら即再生開始
	if( pflag ){ Play(); }
	
	PRINTD( SND_LOG, " SampleRate : %d\n", SampleRate );
	PRINTD( SND_LOG, " BufferSize : %d\n", samples );
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// サンプリングレート取得
//
// 引数:	なし
// 返値:	int			サンプリングレート
/////////////////////////////////////////////////////////////////////////////
int SND6::GetSampleRate( void )
{
	return SampleRate;
}


/////////////////////////////////////////////////////////////////////////////
// バッファサイズ(倍率)取得
//
// 引数:	なし
// 返値:	int			バッファサイズ(倍率)
/////////////////////////////////////////////////////////////////////////////
int SND6::GetBufferSize( void )
{
	return BSize;
}


/////////////////////////////////////////////////////////////////////////////
// マスター音量設定
//
// 引数:	vol			音量(0-100)
// 返値:	なし
/////////////////////////////////////////////////////////////////////////////
void SND6::SetVolume( int vol )
{
	Volume = min( max( vol, 0 ), 100 );
}


/////////////////////////////////////////////////////////////////////////////
// サウンド事前更新関数
//
// 引数:	samples		サンプル数
//			exbuf		外部バッファポインタ
// 返値:	int			更新したサンプル数
/////////////////////////////////////////////////////////////////////////////
int SND6::PreUpdate( int samples, cRing* exbuf )
{
	int exsam = samples;
	
	PRINTD( SND_LOG,"PreUpdate" );
	
	for( auto &p : sdev ){
		PRINTD( SND_LOG," %d", p->ReadySize() );
		exsam = min( exsam, p->ReadySize() );
	}
	PRINTD( SND_LOG,"\n" );
	
	for( int i = 0; i < exsam; i++ ){
		int dat = 0;
		
		for( auto &p : sdev ){
			dat += p->Get();
		}
		
		dat = ( dat * Volume ) / 100;
		dat = min( max( dat, INT16_MIN ), INT16_MAX );
		
		this->cRing::Put( (int16_t)dat );
		// 外部バッファが存在すれば書込み
		if( exbuf ){
			exbuf->Put( (int16_t)dat );
		}
	}
	
	return exsam;
}


/////////////////////////////////////////////////////////////////////////////
// バッファから溢れたサンプル数取得
// 引数:	なし
// 返値:	int			溢れたサンプル数 (溢れていなければ0)
/////////////////////////////////////////////////////////////////////////////
int SND6::OverflowSamples( void )
{
#ifndef NOCALLBACK	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	int size = cRing::ReadySize();
#else				// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	int size = OSD_GetQueuedAudioSamples();
#endif				// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	return size > cRing::GetSize() * MULTI ? size - cRing::GetSize() : 0;
}


#ifndef NOCALLBACK	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/////////////////////////////////////////////////////////////////////////////
// サウンド更新関数(Callback)
//
// 引数:	stream		ストリーム書込みバッファへのポインタ
//			samples		サンプル数
// 返値:	なし
/////////////////////////////////////////////////////////////////////////////
void SND6::Update( BYTE* stream, int samples )
{
	int16_t* str = (int16_t*)stream;
	
	PRINTD( SND_LOG, "[SND6][Update] Stream:%p Samples:%d / %d\n", stream, samples, this->cRing::ReadySize() );
	
	for( int i = 0; i < samples; i++ ){
		*(str++) = (int16_t)this->cRing::Get();
	}
}


#else				// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/////////////////////////////////////////////////////////////////////////////
// サウンド更新関数(Push)
//
// 引数:	なし
// 返値:	なし
/////////////////////////////////////////////////////////////////////////////
void SND6::Update( void )
{
	PRINTD( SND_LOG, "[SND6][Update] " );
	
	// キューがいっぱいならスキップ
	int ofsize = OverflowSamples();
	if( ofsize > 0 ){
		PRINTD( SND_LOG, "<Overflow> %d\n", ofsize );
		return;
	}
	
	int samples = this->cRing::ReadySize();
	
	PRINTD( SND_LOG, "Samples:%d\n", samples );
	
	std::vector<int16_t> stream;
	stream.reserve( samples );
	
	for( int i = 0; i < samples; i++ ){
		stream.push_back((int16_t)this->cRing::Get());
	}
	OSD_WriteAudioStream( reinterpret_cast<BYTE*>(stream.data()), stream.size() );
}
#endif				// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
