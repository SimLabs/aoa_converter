SET(TARGET_H sampler.h dds.h glext.h btx_to_dds.h)
SET(TARGET_SRC ReaderWriterBTX.cpp sampler.cpp btx_to_dds.cpp)
#### end var setup  ###
SETUP_PLUGIN(btx)
