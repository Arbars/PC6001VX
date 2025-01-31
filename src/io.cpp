/////////////////////////////////////////////////////////////////////////////
//  P C 6 0 0 1 V
//  Copyright 1999,2022 Yumitaro
/////////////////////////////////////////////////////////////////////////////
#include <stdexcept>

#include "error.h"
#include "io.h"
#include "log.h"


// ポート数マスク(必ず右詰め)
#define BANKMASK	0xff


// --------------------------------------------------------------------------
//	IOBus : I/O空間を提供するクラス
//	  Original     : cisc
//	  Modification : Yumitaro
// --------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
IOBus::IOBus()
{
}


/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
IOBus::~IOBus()
{
}


/////////////////////////////////////////////////////////////////////////////
// 初期化
/////////////////////////////////////////////////////////////////////////////
bool IOBus::Init( int banksize )
{
	// INポート初期化(ダミーデバイス割当)
	InBank idummy;
	idummy.device = &dummyio;
	idummy.func   = STATIC_CAST( InFuncPtr, &DummyIO::dummyin );
	ins.assign( banksize, std::vector<InBank>( 1, idummy ) );
	
	// OUTポート初期化(ダミーデバイス割当)
	OutBank odummy;
	odummy.device = &dummyio;
	odummy.func   = STATIC_CAST( OutFuncPtr, &DummyIO::dummyout );
	outs.assign( banksize, std::vector<OutBank>( 1, odummy ) );
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// デバイス接続
/////////////////////////////////////////////////////////////////////////////
// IN/OUT -----------
bool IOBus::Connect( const std::shared_ptr<IDevice>& device, const std::vector<Connector>& connector )
{
	devlist.Add( device );
	
	const IDevice::Descriptor& desc = device->GetDescriptors();
	
	for( auto &i : connector ){
		try{
			switch( i.rule ){
			case portin:
				if( !ConnectIn( i.bank, device, desc.indef.at( i.id ) ) ){
					return false;
				}
				break;
				
			case portout:
				if( !ConnectOut( i.bank, device, desc.outdef.at( i.id ) ) ){
					return false;
				}
				break;
			}
		}
		catch( std::out_of_range& ){
			return false;
		}
	}
	return true;
}


// IN -----------
bool IOBus::ConnectIn( int bank, const std::shared_ptr<IDevice>& device, InFuncPtr func )
{
	PRINTD( IO_LOG, "[IO][ConnectIn] %02XH -> ", bank );
	
	try{
		InBank ib;
		
		ib.device = device.get();
		ib.func   = func;
		ins.at( bank ).emplace_back( ib );
	}
	catch( std::out_of_range& ){
		PRINTD( IO_LOG, "Out of range\n" );
		return false;
	}
	
	PRINTD( IO_LOG, "OK\n" );
	return true;
}


// OUT -----------
bool IOBus::ConnectOut( int bank, const std::shared_ptr<IDevice>& device, OutFuncPtr func )
{
	PRINTD( IO_LOG, "[IO][ConnectOut] %02XH -> ", bank );
	
	try{
		OutBank ob;
		
		ob.device = device.get();
		ob.func   = func;
		outs.at( bank ).emplace_back( ob );
	}
	catch( std::out_of_range& ){
		PRINTD( IO_LOG, "Out of range\n" );
		return false;
	}
	
	PRINTD( IO_LOG, "OK\n" );
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// デバイス切断
/////////////////////////////////////////////////////////////////////////////
bool IOBus::Disconnect( const DeviceList::ID id )
{
	// IN
	for( auto &i : ins ){
		for( auto p = i.end() - 1; p != i.begin(); p-- ){
			if( p->device->GetID() == id ){ i.erase( p ); }
		}
	}
	
	// OUT
	for( auto &i : outs ){
		for( auto p = i.end() - 1; p != i.begin(); p-- ){
			if( p->device->GetID() == id ){ i.erase( p ); }
		}
	}
	
	devlist.Del( id );
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// IN関数
/////////////////////////////////////////////////////////////////////////////
BYTE IOBus::In( int port )
{
	BYTE data = 0xff;
	
	for( auto &i : ins.at( port & 0xff ) ) try{
		data &= (i.device->*i.func)( port );
	}
	catch( std::out_of_range& ){}
	
	return data;
}


/////////////////////////////////////////////////////////////////////////////
// OUT関数
/////////////////////////////////////////////////////////////////////////////
void IOBus::Out( int port, BYTE data )
{
	for( auto &i : outs.at( port & 0xff ) ) try{
		(i.device->*i.func)( port, data );
	}
	catch( std::out_of_range& ){}
}


#ifndef NOMONITOR	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

/////////////////////////////////////////////////////////////////////////////
// 登録済I/Oポートリスト取得
/////////////////////////////////////////////////////////////////////////////
void IOBus::GetPortList( std::vector<int>& inp, std::vector<int>& outp )
{
	inp.clear();
	outp.clear();
	
	int i = 0;
	for( auto &p : ins ){
		if( p.size() > 1 ){
			inp.emplace_back( i );
		}
		i++;
	}
	
	i = 0;
	for( auto &p : outs ){
		if( p.size() > 1 ){
			outp.emplace_back( i );
		}
		i++;
	}
}

#endif				// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@




// --------------------------------------------------------------------------
//	DummyIO
// --------------------------------------------------------------------------
IOBus::DummyIO IOBus::dummyio;

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
IOBus::DummyIO::DummyIO( void ) : Device( nullptr, 0 )
{
}


/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
IOBus::DummyIO::~DummyIO()
{
}

/////////////////////////////////////////////////////////////////////////////
// ダミーデバイス(IN)
/////////////////////////////////////////////////////////////////////////////
BYTE IOBus::DummyIO::dummyin( int )
{
	return 0xff;
}


/////////////////////////////////////////////////////////////////////////////
// ダミーデバイス(OUT)
/////////////////////////////////////////////////////////////////////////////
void IOBus::DummyIO::dummyout( int, BYTE )
{
	return;
}








/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
IO6::IO6( void )
{
}


/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
IO6::~IO6( void )
{
}


/////////////////////////////////////////////////////////////////////////////
// 初期化
/////////////////////////////////////////////////////////////////////////////
bool IO6::Init( int banksize )
{
	PRINTD( IO_LOG, "[IO][Init]\n" );
	
	// オブジェクト確保
	try{
		if( !IOBus::Init( banksize ) ){
			throw Error::InitFailed;
		}
		
		Iwait.assign( BANKMASK + 1, 0 );
		Owait.assign( BANKMASK + 1, 0 );
		Idata.assign( BANKMASK + 1, -1 );
		Odata.assign( BANKMASK + 1, -1 );
	}
	catch( Error::Errno i ){	// 例外発生
		Error::SetError( i );
		return false;
	}
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// IN関数
/////////////////////////////////////////////////////////////////////////////
BYTE IO6::In( int port, int* wcnt )
{
	PRINTD( IO_LOG, "[IO][In] port : %02X\n", port );
	
	if( wcnt ){ (*wcnt) += Iwait[port & BANKMASK]; }
	Idata[port & BANKMASK] = IOBus::In( port );
	return Idata[port & BANKMASK];
}


/////////////////////////////////////////////////////////////////////////////
// OUT関数
/////////////////////////////////////////////////////////////////////////////
void IO6::Out( int port, BYTE data, int* wcnt )
{
	PRINTD( IO_LOG, "[IO][Out] port : %02X  data : %02X\n", port, data );
	
	if( wcnt ){ (*wcnt) += Owait[port & BANKMASK]; }
	Odata[port & BANKMASK] = data;
	IOBus::Out( port, data );
}


/////////////////////////////////////////////////////////////////////////////
// IN ウェイト設定
/////////////////////////////////////////////////////////////////////////////
void IO6::SetInWait( int port, int wait )
{
	Iwait[port & BANKMASK] = wait;
}


/////////////////////////////////////////////////////////////////////////////
// OUTウェイト設定
/////////////////////////////////////////////////////////////////////////////
void IO6::SetOutWait( int port, int wait )
{
	Owait[port & BANKMASK] = wait;
}


/////////////////////////////////////////////////////////////////////////////
// IN ウェイト取得
/////////////////////////////////////////////////////////////////////////////
int IO6::GetInWait( int port )
{
	return Iwait[port & BANKMASK];
}


/////////////////////////////////////////////////////////////////////////////
// IN ウェイト取得
/////////////////////////////////////////////////////////////////////////////
int IO6::GetOutWait( int port )
{
	return Owait[port & BANKMASK];
}


#ifndef NOMONITOR	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

/////////////////////////////////////////////////////////////////////////////
// IN データ参照
/////////////////////////////////////////////////////////////////////////////
int IO6::PeepIn( int port )
{
	return Idata[port & BANKMASK];
}


/////////////////////////////////////////////////////////////////////////////
// OUTデータ参照
/////////////////////////////////////////////////////////////////////////////
int IO6::PeepOut( int port )
{
	return Odata[port & BANKMASK];
}

#endif				// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
