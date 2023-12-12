# `fuzz-helper` afl-fuzz和libfuzzer模糊测试助手
   + `readbit.c`   位流读取器
   + `memory.c`    空间主动释放，减少fuzz误报
   + `cvector.c`   用C写的类似STL vector的工具类
   + `libfuzzer.a` Ubuntu下编译的libfuzzer库，可直接和fuzz harness编译

   