@echo off
call tools\build.bat virtio-win.sln "Wxp Wnet Wlh Win7 Win8 Win8.1 Win10" %*
if errorlevel 1 goto :eof
call tools\build.bat NetKVM\NetKVM-VS2015.vcxproj "Win10_SDV" %*
if errorlevel 1 goto :eof
call tools\build.bat vioscsi\vioscsi.vcxproj "Win8_SDV Win10_SDV" %*
if errorlevel 1 goto :eof
call tools\build.bat viostor\viostor.vcxproj "Win8_SDV Win10_SDV" %*
if errorlevel 1 goto :eof

for %%D in (pciserial fwcfg) do (
  pushd %%D
  call buildAll.bat
  if errorlevel 1 goto :eof
  popd
)
