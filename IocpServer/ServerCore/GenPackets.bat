pushd %~dp0

protoc.exe -I=./ --cpp_out=./ ./Protocol.proto

IF ERRORLEVEL 1 PAUSE