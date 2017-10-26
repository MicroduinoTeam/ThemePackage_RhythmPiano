/*****************************************
 * 节奏大师应用程序
 ****************************************/

#include <Microduino_ColorLED.h>  //彩灯库
#include <Microduino_Key.h>       //按键库
#include <Microduino_AudioPro.h>  //audioPro库
#include "Microduino_Music.h"     //音乐库

#define LED_NUM 7                 //定义彩灯数量
#define PIN_LED SDA               //定义彩灯端口号
#define PIN_RANDOM  A6            //定义产生随机数所用端口号
#define MAX_RANDOM  6             //定义随机数最大值
#define maxMissNum 10             //定义最大允许失误次数
#define maxWaitTime 3000          //定义单音级最长等待时间，单位毫秒

/***********定义彩灯颜色*/
const uint32_t colorArray[10] = { 
  COLOR_NONE,       //0x000000, 彩灯关闭，colorLED OFF
  COLOR_RED,        //0xff0000, 红色，Red
  COLOR_ORANGE,     //0xea8b17, 橙色，Orange
  COLOR_YELLOW,     //0xffff00, 黄色，Yellow
  COLOR_GREEN,      //0x00ff00, 绿色，Green
  COLOR_CYAN,       //0x00ffff, 青色，Cyan
  COLOR_BLUE,       //0x0000ff, 蓝色，Blue
  COLOR_PURPLE,     //0xff00ff, 紫色，Purple
  COLOR_WARM,       //0x9b6400, 暖色，Warm
  COLOR_COLD,       //0x648264, 冷色，Cold
};

ColorLED strip = ColorLED(LED_NUM, PIN_LED);

AudioPro midiPlayer;

DigitalKey touchButton[7] = {   //touchButton引脚分配
  DigitalKey(4), DigitalKey(5), DigitalKey(6), DigitalKey(8), DigitalKey(9), DigitalKey(A0), DigitalKey(A6)
};

int playNum = 0;       //挑战曲目号
uint8_t missNum = 0;   //失误次数

void setup() {
  /***********串口初始化*/
  Serial.begin(115200);

  /***********触摸按键初始化*/
  for (int a = 0; a < 7; a++) {
    touchButton[a].begin();
  }

  /*************彩灯初始化*/
  strip.begin();
  strip.setBrightness(80);
  strip.show();
  
  /***********随机引脚初始化*/
  pinMode(PIN_RANDOM, INPUT_PULLUP);
  int seed = analogRead(PIN_RANDOM);
  randomSeed(seed);

  /************audioPro初始化*/
  if (! midiPlayer.begin()) { // initialise the music player
    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    while (1);
  }
  Serial.println(F("VS1053 found"));
  midiPlayer.applyPatch(MIDIPatch, sizeof(MIDIPatch) / sizeof(uint16_t));
  midiPlayer.midiSetVolume(0, 127);
  midiPlayer.midiSetBank(0, VS1053_BANK_DEFAULT);
  midiPlayer.midiSetInstrument(0, VS1053_GM1_ELECTRIC_GRAND_PIANO);
  delay(200);

  /**********彩灯与音乐逐渐点亮与响起*/
  for (int a = 0; a < 7; a++) {
    midiPlayer.noteOn(0, toneNum[a], 120);     //播放相应音级
    strip.setPixelColor(a, colorArray[a + 1]); //设置相应灯的颜色
    strip.show();                              //将灯点亮成新的颜色
    delay(100);
    midiPlayer.noteOff(0, toneNum[a], 120);    //关闭相应音级
    strip.setPixelColor(a, colorArray[0]);     //设置相应灯的颜色,即熄灭灯
    strip.show();                              //将灯点亮成新的颜色
  }
  delay(500);

  /***********彩灯与音乐同时点亮与响起*/
  for (int a = 0; a < 7; a++) {
    midiPlayer.noteOn(0, toneNum[a], 120);      //播放相应音级
    strip.setPixelColor(a, colorArray[a + 1]);  //设置相应灯的颜色
    strip.show();                               //将灯点亮成新设置的颜色
  }
  delay(1000);

  /************关闭音乐*/
  for (int a = 0; a < 7; a++) {
    midiPlayer.noteOff(0, toneNum[a], 120);     //关闭相应音级
    delay(20);
    strip.setPixelColor(a, colorArray[0]);     //设置相应灯的颜色,即熄灭灯
    strip.show();                              //将灯点亮成新的颜色
  }

  /**********产生随机曲目号*/
  playNum = random(MAX_RANDOM);
  Serial.print("random playNum: ");           //串口打印随机曲目号
  Serial.println(playNum);

}

void loop() {
  boolean missSta;  //游戏是否失误状态变量
  missSta = 0;
  
  /**********进入游戏*/
  for (int i = 0; i < notationNum[playNum]; i++) {     //挑战游戏
    missSta = notationShow(notationName[playNum], i);  //判断按下的按键与预设音符是否匹配

    /**********如果按下的按键与预设音符不匹配*/
    if (missSta == false) {
      missNum++;                  //失误数加1
      Serial.print("missNum:");
      Serial.println(missNum);
    }

    /**********如果失误数大于允许的最大失误数*/
    if (missNum > maxMissNum) {
      Serial.println("GAMEOVER");                 //串口打印"GAMEOVER"
      for (int a = 0; a < 7; a++) {               //依次播放音级与点亮彩灯
        midiPlayer.noteOn(0, toneNum[a], 120);    //播放相应音级
        strip.setPixelColor(a, colorArray[a + 1]);//设置相应灯的颜色
        strip.show();                             //将灯点亮成新的颜色
        delay(500);                               //延时500ms
      }
      while ((touchButton[0].readVal() == KEY_RELEASED) || (touchButton[1].readVal() == KEY_RELEASED) || (touchButton[2].readVal() == KEY_RELEASED)); //等待按下前三个按钮
      for (int a = 0; a < 7; a++) {
        midiPlayer.noteOff(0, toneNum[a], 120);    //关闭相应音级
        strip.setPixelColor(a, colorArray[0]);     //设置相应灯的颜色，即熄灭灯
        strip.show();                              //将灯点亮成新设置的颜色
      }
      break;
    }
  }

  /**********如果挑战成功,播放挑战成功曲目，彩灯随音乐亮灭，音乐播放完毕后产出下一挑战曲目号*/
  int playSta = 0;  //定义临时变量playSta，用来计算彩灯号
  if (missNum <= maxMissNum) {                         //如果失误次数小于等于允许的最大失误次数则挑战成功
    Serial.println("successful");                     //串口打印"successful"
    
    /**********完整播放一遍挑战曲目*/
    for (int i = 0; i < notationNum[playNum]; i++) {   //循环次数为对应曲目的音阶数
      playSta = (notationName[playNum][i] - 1) % 7;    //计算当前音阶对应的彩灯号
      midiPlayer.noteOn(0, toneNum[notationName[playNum][i] - 1], 120); //播放相应音阶
      strip.setPixelColor(playSta, colorArray[playSta + 1]);             //设置相应灯的颜色
      strip.show();                                                      //将灯点亮成新设置的颜色
      delay(30 * (beatsName[playNum][i]));                             //延时节拍相对应时间
      midiPlayer.noteOff(0, toneNum[notationName[playNum][i] - 1], 120);//关闭相应音阶
      strip.setPixelColor(playSta, colorArray[0]);                       //设置相应灯的颜色，即熄灭灯
      strip.show();                                                      //将灯点亮成新设置的颜色
      delay(60);
    }
    
    /**********曲目播放完成后，产生新的曲目号*/
    playNum++;            //曲目号加1
    if (playNum >= 5) {   //如果曲目号大于5
      playNum = 0;        //曲目号等于0
    }
  }

  /**********将错误数归0*/
  missNum = 0;
}


/**********************************************************
 * 此函数用来判断有无正确按下游戏中预设音符所对应的触摸按键，
 * 正确按下返回true，按错或在设定时间内没操作返回false
*********************************************************/
boolean notationShow(uint8_t *notation, uint8_t i)
{
  uint32_t timer = millis();
  /**********点亮预设音符所对应彩灯*/
  uint8_t notaSta = (notation[i] - 1) % 7;                //计算当前音阶所对应的彩灯号
  strip.setPixelColor(notaSta, colorArray[notaSta + 1]);  //设置相应灯的颜色
  strip.show();                                           //将灯点亮成新设置的颜色

  /**********在设定时间内判断按键按下状态*/
  while (millis() - timer < maxWaitTime) {  
    for (int a = 0; a < 7; a++) {             //循环判断有无触摸按键按下
      uint8_t b = touchButton[a].readVal();   //将触摸按键状态值赋给临时变量b

      if (b == KEY_PRESSED) { //如果有触摸按键按下

        /**********如果按键选择与当前音阶编号相同，即选择正确，执行以下语句
         ***********播放正确音级，熄灭灯，串口打印正确提示字符，返回正确状态*/
        if (a == notaSta) {                 //如果按键选择正确
          midiPlayer.noteOn(0, toneNum[notation[i] - 1], 120);   //播放相应音级
          while (touchButton[notaSta].readVal() == KEY_PRESSED); //等待按键松开
          strip.setPixelColor(notaSta, colorArray[0]);           //设置相应灯的颜色,即熄灭灯
          strip.show();                                          //将灯点亮成新设置的颜色
          delay(60);
          Serial.println("true_OK");                            //串口打印"true_OK"
          return true;                                           //跳出整个函数，返回值为true
        }

        /**********如果按键选择错误，执行以下语句
        ***********播放所选的错误音级，熄灭灯，串口打印错误提示字符，返回错误状态*/
        midiPlayer.noteOn(0, toneNum[a], 120);            //播放所选音阶
        while (touchButton[a].readVal() == KEY_PRESSED);  //等待按键松开
        strip.setPixelColor(notaSta, colorArray[0]);      //设置相应灯的颜色,即熄灭灯
        strip.show();                                     //将灯点亮成新设置的颜色
        Serial.print(a);                                 //串口打印当前按下触摸按键编号a
        Serial.println("false_OK");                      //串口打印"false_OK"
        return false;                                     //跳出整个函数，返回值为false
      }
    }
  }

  /**********如果在等待时间内无操作，执行以下语句
  ***********熄灭灯，返回错误状态*/
  strip.setPixelColor(notaSta, colorArray[0]);  //设置相应灯的颜色,即熄灭灯
  strip.show();                                 //将灯点亮成新设置的颜色
  return false;       //跳出函数，返回值为false
}

