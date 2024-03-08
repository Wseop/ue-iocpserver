pushd %~dp0

protoc.exe -I=./ --cpp_out=./ ./Protocol.proto

IF ERRORLEVEL 1 PAUSE

XCOPY /Y Protocol.pb.h "../../UE_Client/Source/UE_Client/ServerCore"
XCOPY /Y Protocol.pb.cc "../../UE_Client/Source/UE_Client/ServerCore"