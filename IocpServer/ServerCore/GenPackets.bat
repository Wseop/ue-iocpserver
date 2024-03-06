pushd %~dp0

protoc.exe -I=./ --cpp_out=./ ./Enum.proto

IF ERRORLEVEL 1 PAUSE