echo "Building ASMRunner"
call "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"
msbuild Tests\ASMRunner\ASMRunner.vcxproj /t:Rebuild /p:Configuration=Debug
Tests\ASMRunner\Debug\ASMRunner.exe
