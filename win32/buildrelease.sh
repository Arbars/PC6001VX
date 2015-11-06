#���̃X�N���v�g�̒u���ꏊ���J�����g�Ƃ��Ď��s���邱�ƁB
#�J�����g�f�B���N�g��
export SCRIPT_DIR=$PWD

#����r���h
MINGW32MAKE="mingw32-make -j$NUMBER_OF_PROCESSORS"

P6VX_DBUILD_DIR=PC6001VX-build

pushd $SCRIPT_DIR/../../
if [ -e $P6VX_DBUILD_DIR ]; then 
	rm -rf $P6VX_DBUILD_DIR
fi
mkdir $P6VX_DBUILD_DIR

cd $P6VX_DBUILD_DIR
/mingw32/local/bin/qmake ../PC6001VX/PC6001VX.pro 
$MINGW32MAKE debug
$MINGW32MAKE release

