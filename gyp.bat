set GYP_MSVS_VERSION=2010
set GYP_DEFINES=disable_nacl=1 use_ozone=0 use_aura=0 skia_support_pdf=0 windows_sdk_path="C:\Program Files\Windows Kits\8.0"
build\tools\python27\python.exe build\gyp_chromium
