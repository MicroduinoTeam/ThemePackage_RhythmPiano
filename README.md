﻿# ThemePackage_RhythmPiano

==============
2017-07-14：

Arduino IDE 1.6.9 for Microduino的Library进行了一次大更新，可能会和之前的程序有不兼容

此程序适配于更新后的IDE

核心要使用core+

功能说明：
在开机时灯依次亮起并播放“1 2 3 4 5 6 7”，然后逐个灭掉，在所有灯灭掉之前按下Touch，选择挑战的曲子，如果未按下则随机产生挑战曲子；
ColorLED按照预设“音符”按下相应的Touch,每个“音符”有效按下时间为3S，超过3S未按下或者按错则记为一次失误，一次挑战中出现十次失误则为挑战失败，小于10次即为成功。
挑战失败：从第一个到第七个灯逐个亮起并播放“1 2 3 4 5 6 7”，之后保持灯全亮（前三个键同时按下解锁）；
挑战成功：自动播放一遍曲子。然后开始下一首曲子。