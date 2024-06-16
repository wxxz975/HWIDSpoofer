@echo off
echo ========== System Information ==========

echo [CPU Information]
wmic cpu get Name, ProcessorId, UniqueId

echo [Motherboard Information]
wmic baseboard get Product, Manufacturer, Version, SerialNumber

echo [SMBIOS Information]
wmic bios get Manufacturer, Name, Version, SerialNumber

echo [Disk Information]
wmic diskdrive get Model, SerialNumber

echo [Network Adapter Information]
wmic nic get Name, MACAddress, GUID

echo [Memory Information]
wmic memorychip get BankLabel, Capacity, Manufacturer, PartNumber, SerialNumber

echo [GPU Information]
nvidia-smi --query-gpu=gpu_name,uuid --format=csv

echo ========== End of System Information ==========
pause