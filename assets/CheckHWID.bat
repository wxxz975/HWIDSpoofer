@echo off 
title 计算机硬件信息查询
color 0B
mode con cols=79 lines=45
cls
echo. 
echo.      _______________机器码硬件检测________________
echo.
echo.    主版:
for /f "tokens=* delims=" %%a in ('reg query "HKLM\HARDWARE\DESCRIPTION\System\BIOS" /v "SystemManufacturer"') do (set "Brand=%%a")
echo.            品牌：%Brand:~36%
for /f "tokens=* delims=" %%a in ('reg query "HKLM\HARDWARE\DESCRIPTION\System\BIOS" /v "BaseBoardProduct"') do (set "model=%%a")
echo.            型号：%model:~34%
)
echo.
echo.名字
wmic csproduct get Name
echo.硬盘物理序列号___
wmic diskdrive get serialnumber
echo.主板CPU序列号___
wmic cpu get serialnumber
echo.主板biso序列号___
wmic bios get serialnumber
echo.主板物理序列号___
wmic baseboard get serialnumber
echo.主板UUID
wmic csproduct get uuid

echo.网卡信息MAC
@echo off&color 0B&&setlocal EnableDelayedExpansion
for /f "tokens=*" %%i in ('ipconfig /all^|findstr /i "描述 物理地址"') do set "qq=%%i"&&set "qq=!qq:. =!"&&echo.!qq!
echo.
for /f "tokens=*" %%i in ('ipconfig /all^|findstr /i "DUID"') do set "qq=%%i"&&set "qq=!qq:. =!"&&echo.!qq!
getmac
title 查询完毕！
echo.
echo.检测完成,按回车结束。
pause>nul
goto exit



