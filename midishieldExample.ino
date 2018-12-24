/*
  命名规则
  midiSheild有关的函数都是关于midiSheild有关的配置
  midiPlayer有关的函数都是关于midiPlayer有关的配置
*/


//引入所有需要的库文件
#include <SoftwareSerial.h>
#include <MsTimer2.h>
#include <MIDI.h>

//分别代表了频率、音量和声道，是整数哦
//都是一些全局不变变量，不用修改
int freq, velocity, mainChannel = 0;
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);
SoftwareSerial mySerial(10, 11);

//乐器序号在这里修改
int  instrument = 0;


//已经将程序封装好了，分别代表按下和松开触发的代码
void midiOnAction() {
  noteOn(mainChannel, freq, velocity);
}

void midiOffAction() {
  noteOff(mainChannel, freq, 0);
}



void setup()
{

  midiSheildSetup();
  midiPlayerSetup();

}

void midiSheildSetup() {
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOn();
}
void midiPlayerSetup()
{
  byte resetMIDI = 12; //数字引脚4，连接到VS1053的复位引脚
  pinMode(resetMIDI, OUTPUT);   //复位引脚设置为输出
  digitalWrite(resetMIDI, LOW); //引脚电平拉低
  delay(100);
  digitalWrite(resetMIDI, HIGH);//引脚电平拉高
  delay(100);
  talkMIDI(0xB0, 0x07, 120);    //0xB0 是通道消息, 设置通道音量接近最大 (127)
  mySerial.begin(31250);
}

void loop()
{
  midiRead();
}

void midiRead() {
  if (MIDI.read())
  {
    switch (MIDI.getType())
    {
      case midi::NoteOff :
        {
          freq = MIDI.getData1();
          velocity = MIDI.getData2();
          midiOffAction();
        }
        break;
      case midi::NoteOn :
        {
          //
          freq = MIDI.getData1();
          velocity = MIDI.getData2();
          midiOnAction();
        }
        break;
    }
  }
}
//midiPlayer函数
void noteOn(byte channel, byte note, byte attack_velocity) {
  talkMIDI( (0x90 | channel), note, attack_velocity);
}
void noteOff(byte channel, byte note, byte release_velocity) {
  talkMIDI( (0x80 | channel), note, release_velocity);
}
void talkMIDI(byte cmd, byte data1, byte data2) {
  mySerial.write(cmd);
  mySerial.write(data1);

  //Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes
  //有一些命令只有一个字节。所有小于0xBn的命令有两个数据字节
  //(sort of: http://253.ccarh.org/handout/midiprotocol/)
  if ( (cmd & 0xF0) <= 0xB0)
    mySerial.write(data2);
}
