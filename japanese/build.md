# JNetHack�r���h�菇

NetHack��JNetHack�̃p�b�`�𓖂Ă���ԁA���邢��git���|�W�g������clone������Ԃ���J�n���܂��B�J�����g�f�B���N�g���̓x�[�X�f�B���N�g��(`/src`, `/dat` �Ȃǂ�����f�B���N�g��)�Ƃ��܂��B

���̏�Ԃł͕����R�[�h��Shift_JIS�ɂȂ��Ă��܂����AOS�ɂ���ăr���h���邽�߂̕����R�[�h���قȂ�܂��B�ȉ��̎菇�ł͕����R�[�h�ϊ��������܂�ł��܂��B

## Windows

VisualStudio 2017�ɑΉ����Ă��܂��B�u�J���҃R�}���h �v�����v�g for VS2017�v������s���܂��B

```
sys/winnt/setup.bat
cd src
nmake install
```

## Linux

NetHack�̃r���h���ɉ�����`nkf`���K�v�ł��B

```
sh japanese/set_lnx.sh
make install
```

## MacOS

NetHack�̃r���h���ɉ����� `GCC@5�ȏ�`, `nkf` ���K�v�ł��BXCode�Ɋ܂܂�Ă���`gcc`�ł̓r���h�ł��܂���̂ŁAhomebrew�ŃC���X�g�[�����Ă��������B

�ݒ�t�@�C����GCC@8��O��Ƃ��ď�����Ă��܂��B����ȊO�̃o�[�W�����̏ꍇ��`sys/unix/hints` �� `japanese/set_mac.sh` �̏C�����K�v�ł��B

```
sh japanese/set_mac.sh
make install
```
