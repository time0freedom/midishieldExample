#include <SoftwareSerial.h>
#include <MsTimer2.h>
#include <MIDI.h>

#define PIN_RAW_INPUT 2

#define PIN_POT_A0 0
#define PIN_POT_A1 1

static const uint16_t DEBOUNCE_COUNT = 50;

SoftwareSerial mySerial(10, 11);//引脚2是RX（没用到）, 引脚3是TX

byte note = 0;      //音符变量，表示将要播放的音符，初始值是0，范围是0-127
byte resetMIDI = 12; //数字引脚4，连接到VS1053的复位引脚
byte ledPin = 13;   //MIDI通讯指示灯
int  instrument = 0;//乐器号变量，初始值为0


MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);


void setup()
{

  // ============[MIDI receiver board]=============

  // LED outputs
  pinMode(8, OUTPUT);
  pinMode(A0, INPUT);
  digitalWrite(8, HIGH);
  // do I need to init the soft serial port?
  // No - MIDI Lib will do it.

  // We want to receive messages on all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);

  // We also want to echo the input to the output,
  // so the sniffer can be dropped inline when things misbehave.
  MIDI.turnThruOn();

  pinMode(PIN_RAW_INPUT, INPUT_PULLUP);
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);



  // ============[MIDI player board]=============

  mySerial.begin(31250);

  //对VS1053进行复位
  pinMode(resetMIDI, OUTPUT);   //复位引脚设置为输出
  digitalWrite(resetMIDI, LOW); //引脚电平拉低
  delay(100);
  digitalWrite(resetMIDI, HIGH);//引脚电平拉高
  delay(100);
  talkMIDI(0xB0, 0x07, 120);    //0xB0 是通道消息, 设置通道音量接近最大 (127)

}

void loop()
{
  static uint8_t  ticks = 0;
  static uint8_t  old_ticks = 0;

  talkMIDI(0xB0, 0, 0x00); //默认bank GM1
  talkMIDI(0xC0, 4, 0);//1-8 piano


  // turn the crank...
  if (  MIDI.read())
  {
    switch (MIDI.getType())
    {
      case midi::NoteOff :
        {
          int a = MIDI.getData1();
          int b = MIDI.getData2();
          noteOff(0, a, b);
        }
        break;
      case midi::NoteOn :
        {
          //
          int freq = MIDI.getData1();
          int vel = MIDI.getData2();
          noteOn(0, a, b);
          //·············你们的舵机操作就在这里················//
          
          //················································//
          
        }
        break;
    }
  }

}


//发送一个MIDI音符开消息
//通道范围0-15
void noteOn(byte channel, byte note, byte attack_velocity) {
  talkMIDI( (0x90 | channel), note, attack_velocity);
}

//发送一个MIDI音符关消息
void noteOff(byte channel, byte note, byte release_velocity) {
  talkMIDI( (0x80 | channel), note, release_velocity);
}

//播放一个音符
//不要视图去看一看数值大于127或者小于127是什么样的
void talkMIDI(byte cmd, byte data1, byte data2) {
  digitalWrite(ledPin, HIGH);
  mySerial.write(cmd);
  mySerial.write(data1);

  //Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes
  //有一些命令只有一个字节。所有小于0xBn的命令有两个数据字节
  //(sort of: http://253.ccarh.org/handout/midiprotocol/)
  if ( (cmd & 0xF0) <= 0xB0)
    mySerial.write(data2);
  digitalWrite(ledPin, LOW);
}
