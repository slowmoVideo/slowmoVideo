.PHONY : ffmpegTest
ffmpegTest :
	gcc -g -Wall -std=c99 -o ffmpegTest ffmpegTest.c -D__STDC_CONSTANT_MACROS -lavcodec -lavformat

.PHONY : encode
encode :
	gcc -g -Wall -std=c99 -o ffmpegEncodeTest ffmpegTestEncode.c -D__STDC_CONSTANT_MACROS -lavcodec -lavformat