@echo off

set CODEGEN=tools\codegen\codegen.exe

@echo *** generating OpenGL apis ***
call %CODEGEN% -h templates\oglapis_h.header -f templates\oglapis_h.footer -b templates\oglapis_h.body -d templates\ogldefs.csv -o include\insigne\generated_code\oglapis.h
call %CODEGEN% -h templates\oglapis_cpp.header -f templates\oglapis_cpp.footer -b templates\oglapis_cpp.body -d templates\ogldefs.csv -o src\generated_code\oglapis.cpp

@echo *** generating OpenGL proxy functions ***
call %CODEGEN% -h templates\proxy_h.header -f templates\proxy_h.footer -b templates\proxy_h.body -d templates\ogldefs.csv -o include\insigne\generated_code\proxy.h
call %CODEGEN% -h templates\proxy_cpp.header -f templates\proxy_cpp.footer -b templates\proxy_cpp_v0.body -b templates\proxy_cpp_v1.body -d templates\ogldefs.csv -o src\generated_code\proxy.cpp

@echo *** generating OpenGL api helpers ***
call %CODEGEN% -h templates\helpers.header -b templates\helpers.body -d templates\ogldefs.csv -o include\insigne\generated_code\helpers.h
