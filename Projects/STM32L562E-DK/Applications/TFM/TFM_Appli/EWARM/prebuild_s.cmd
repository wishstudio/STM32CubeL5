iccarm.exe --cpu=Cortex-M33 -DBL2 -DMCUBOOT_IMAGE_NUMBER=2 -DTFM_PSA_API -I%1\..\..\Linker  %1\tfm_common_s.icf  --silent --preprocess=ns %1\tfm_common_s.icf.i 2>&1
iccarm.exe --cpu=Cortex-M33 -DBL2 -DMCUBOOT_IMAGE_NUMBER=2 -DTFM_PSA_API -I%1\..\..\Linker  %1\..\..\TFM_SBSFU\Src\image_macros_to_preprocess_s.c --silent --preprocess=ns %1\image_macros_preprocessed_s.c 2>&1