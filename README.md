# SIMD
## 代码简介
> 代码依赖：提前下载yuvplayer
* yuv目录中的dem1.yuv和dem2.yuv是两个单帧的yuv文件,lab 4.1用这两个就行,大小都是1920*1080(width*height),截取于parkscene.yuv文件;
* parkscene.yuv是一个240帧的yuv文件.yuv文件可以用yuvplayer播放.
* lab4.1两个程序应该分别生成两个84帧的yuv文件(题目要求中的85应该改成84),用yuvplayer播放应该可以看到每一帧渐变的效果.
* lab4.1应该统计出加速处理前所用的时间,以及分别用mmx,sse2,avx加速处理后所用的时间,并对比分析.
* yuv.cpp为4.1的模板,仅供参考.
## lab4.1
### 代码介绍
* yuvFIFO.cpp: fade in fade out
* yuvMIX.cpp: mix two yuv images
### 运行模式
* 0: no SIMD
* 1: SSE
* 2: SSE2
* 3: AVX
## lab4.2
### P集
| funct7  | rs2  | rs1 |     funct3     |  rd |  opcode  |      ins     |
| :-----: |:---: | ---:| :-------------:| ---:| :-------:| :-----------:|
| 0000001 |      |     |     000/001    |     | 1111011  |    PCKWB/DW  |
| 0000010 |      |     |     000/001    |     | 1111011  |   UNPCKBW/WD |
| 0000100 |      |     |   000/001/010  |     | 1111011  |   PADDB/W/D  |
| 0001000 |      |     |   000/001/010  |     | 1111011  |   PSUBB/W/D  |
| 0010000 |      |     |   000/001/010  |     | 1111011  |   PMULB/W/D  |
| 0100000 |      |     |   000/001/010  |     | 1111011  |   PDIVB/W/D  |
| 1000000 |      |     |   000/001/010  |     | 1111011  |   PSLLB/W/D  |
| 1000001 |      |     |   000/001/010  |     | 1111011  |   PSRLB/W/D  | 
| 1000010 |      |     |   000/001/010  |     | 1111011  |   PSRAB/W/D  |

|    IMM[11:0]   | rs1 | funct3         | rd  | opcode   | ins          |
| :-----------:  | --- | :---------:    | --  | :-------:| :----------: |
|                |     | 000/010/011    |     | 1101011  | LDI/II/III   |

|IMM[11:5] | rs2 | rs1 | funct3         | IMM[4:0] | opcode   | ins        |
|----------|-----|---- | :-----------:  |----      | :-------:| :--------: |
|          |     |     | 000/010/011    |----      | 1110111  | STI/II/III |
