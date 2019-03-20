//������ʱ�Ӳ����ڲ�RC������     DCO��8MHz,��CPUʱ��;  SMCLK��1MHz,����ʱ��ʱ��
#include <msp430g2553.h>
#include <tm1638.h>  //��TM1638�йصı���������������ڸ�H�ļ���

//////////////////////////////
//         ��������         //
//////////////////////////////

//��������ʾ����ȼ�
#define GAIN_STATENUM   15
// 0.1s�����ʱ�����ֵ��5��20ms
#define V_T100ms    5
// 0.5s�����ʱ�����ֵ��25��20ms
#define V_T500ms    25
//���ӵ��¶��壬��ʾ�������Ϊ��������Ĺ���
//CD4066�����ź�
#define CTL0_L P1OUT &= ~BIT0;
#define CTL0_H P1OUT |= BIT0;
#define CTL1_L P1OUT &= ~BIT1;
#define CTL1_H P1OUT |= BIT1;
#define CTL2_L P1OUT &= ~BIT2;
#define CTL2_H P1OUT |= BIT2;
#define CTL3_L P1OUT &= ~BIT3;
#define CTL3_H P1OUT |= BIT3;
//����
//����������ɫ������ {Ƶ��ֵ������ֵ}  const����ָ��Ҫ�����ROM��
const unsigned int music_data[][2]=
{
    {523,600},{784,200},{523,200},{784,200},{523,200},{587,200},{659,1600},
    {523,600},{784,200},{523,200},{784,200},{523,200},{587,200},{587,1600},
    {523,600},{784,200},{523,200},{784,200}, {587,200} ,{523,200},{440,1000},{392,200}, {523,200},{587,200},
    {523,600},{784,200},{523,200},{784,200},{523,200},{440,200},{523,1600},
    {523,200},{523,400},{440,200},{392,400},{440,400},
    {523,400},{523,200},{587,200},{659,800},
    {587,200},{587,400},{523,200},{587,400},{587,200},{784,200},
    {784,200},{659,200},{659,200},{587,200},{659,800},
    {523,200},{523,400},{440,200},{392,400},{784,400},
    {659,200},{587,200}, {659,200},{587,200},{523,800},
    {587,200}, {587,400}, {523,200}, {587,200}, {587,400},{659,200},
    {587,200},{523,200},{440,200}, {587,200},{523,800},
    {523,200},{523,400},{440,200},{392,400},{440,400},
    {523,200},{523,400},{587,200},{659,800},
    {587,200},{587,400},{523,200},{587,400},{587,200},{784,200},
    {784,200},{659,200},{659,200},{587,200},{659,800},
    {523,200},{523,200},{523,200},{440,200},{392,400},{784,400},
    {659,200},{587,200}, {659,200},{587,200},{523,800},
    {587,200},{587,400}, {523,200}, {587,200}, {587,400},{659,200},
    {587,200},{523,200},{440,200}, {587,200},{523,800},
    {659,200},{784,400}, {784,200}, {784,400}, {784,400},
    {880,200},{784,200},{659,200},{587,200},{523,800},
    {880,200},{1047,200},{880,200},{784,200},{659,200},{587,200},{523,200},{440,200},
    {587,400},{587,200},{659,200},{659,200},{587,600},
    {659,200},{784,400}, {784,200}, {784,400}, {784,400},
    {880,200},{784,200},{659,200},{587,200},{523,800},
    {440,200},{523,200},{440,200},{392,200},{587,400},{659,400},
    {523,1200},{0,400},
    {0,0}

};

//////////////////////////////
//       ��������           //
//////////////////////////////

unsigned char gain_state=1;
unsigned char key_state=0,key_flag=1,key_code=0,key_timer=0;
// �����ʱ������
unsigned char clock100ms=0;
unsigned char clock500ms=0;
// �����ʱ�������־
unsigned char clock100ms_flag=0;
unsigned char clock500ms_flag=0;
// �����ü�����
unsigned int test_counter=0;
// 8λ�������ʾ�����ֻ���ĸ����
// ע����������λ�������������Ϊ4��5��6��7��0��1��2��3
unsigned char digit[8]={' ','-',' ',' ','G','A','I','N'};
// 8λС���� 1��  0��
// ע����������λС����������������Ϊ4��5��6��7��0��1��2��3
unsigned char pnt=0x04;
// 8��LEDָʾ��״̬��ÿ����4����ɫ״̬��0��1�̣�2�죬3�ȣ���+�̣�
// ע������ָʾ�ƴ������������Ϊ7��6��5��4��3��2��1��0
//     ��ӦԪ��LED8��LED7��LED6��LED5��LED4��LED3��LED2��LED1
unsigned char led[]={0,0,0,0,0,0,0,0};
//��Ƶ��������
unsigned int audio_frequency=0,audio_dura=0,audio_ptr=0;
//////////////////////////////
//       ϵͳ��ʼ��         //
//////////////////////////////

//  I/O�˿ں����ų�ʼ��
void Init_Ports(void)
{
    P2SEL &= ~(BIT7+BIT6);       //P2.6��P2.7 ����Ϊͨ��I/O�˿�
      //������Ĭ�������⾧�񣬹�����޸�

    P2DIR |= BIT7 + BIT6 + BIT5; //P2.5��P2.6��P2.7 ����Ϊ���
      //����·������������������ʾ�ͼ��̹�����TM1638������ԭ�������DATASHEET
    P1DIR |= BIT0 + BIT1 + BIT2 +BIT3;
    P2SEL |= BIT1;
    P2DIR |= BIT1;
 }

//  ��ʱ��TIMER��ʼ����ѭ����ʱ20ms
void Init_Timer(void)
{
    TA0CTL = TASSEL_2 + MC_1 ;      // Source: SMCLK=1MHz, UP mode,
    TA0CCR0 = 20000;                // 1MHzʱ��,����20000��Ϊ 20ms
    TA0CCTL0 = CCIE;                // TA0CCR0 interrupt enabled
    //initial timer1 with pwm
    TA1CTL = TASSEL_2 + MC_1 ;
    TA1CCTL1 = OUTMOD_7;
    TA1CCR0 = 1000000/440;
    TA1CCR1 = TA1CCR0/2;
    TA1CCR0 = 1000000/audio_frequency;
    TA1CCR1 = TA1CCR0/2;
    TA1CTL = TASSEL_2 + MC_1;

}


//  MCU������ʼ����ע���������������
void Init_Devices(void)
{
    WDTCTL = WDTPW + WDTHOLD;     // Stop watchdog timer��ͣ�ÿ��Ź�
    if (CALBC1_8MHZ ==0xFF || CALDCO_8MHZ == 0xFF)
    {
        while(1);            // If calibration constants erased, trap CPU!!
    }

    //����ʱ�ӣ��ڲ�RC������     DCO��8MHz,��CPUʱ��;  SMCLK��1MHz,����ʱ��ʱ��
    BCSCTL1 = CALBC1_8MHZ;   // Set range
    DCOCTL = CALDCO_8MHZ;    // Set DCO step + modulation
    BCSCTL3 |= LFXT1S_2;     // LFXT1 = VLO
    IFG1 &= ~OFIFG;          // Clear OSCFault flag
    BCSCTL2 |= DIVS_3;       //  SMCLK = DCO/8

    Init_Ports();           //���ú�������ʼ��I/O��
    Init_Timer();          //���ú�������ʼ����ʱ��0
    _BIS_SR(GIE);           //��ȫ���ж�
   //all peripherals are now initialized
}

void gain_control(void){
    switch (gain_state){
    case 1:CTL3_L;CTL2_L;CTL1_L;CTL0_H;break;
    case 2:CTL3_L;CTL2_L;CTL1_H;CTL0_L;break;
    case 3:CTL3_L;CTL2_L;CTL1_H;CTL0_H;break;
    case 4:CTL3_L;CTL2_H;CTL1_L;CTL0_L;break;
    case 5:CTL3_L;CTL2_H;CTL1_L;CTL0_H;break;
    case 6:CTL3_L;CTL2_H;CTL1_H;CTL0_L;break;
    case 7:CTL3_L;CTL2_H;CTL1_H;CTL0_H;break;
    case 8:CTL3_H;CTL2_L;CTL1_L;CTL0_L;break;
    case 9:CTL3_H;CTL2_L;CTL1_L;CTL0_H;break;
    case 10:CTL3_H;CTL2_L;CTL1_H;CTL0_L;break;
    case 11:CTL3_H;CTL2_L;CTL1_H;CTL0_H;break;
    case 12:CTL3_H;CTL2_H;CTL1_L;CTL0_L;break;
    case 13:CTL3_H;CTL2_H;CTL1_L;CTL0_H;break;
    case 14:CTL3_H;CTL2_H;CTL1_H;CTL0_L;break;
    case 15:CTL3_H;CTL2_H;CTL1_H;CTL0_H;break;
    }
}

//////////////////////////////
//      �жϷ������        //
//////////////////////////////

// Timer0_A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void)
{
    // 0.1������ʱ������
    if (++clock100ms>=V_T100ms)
    {
        clock100ms_flag = 1; //��0.1�뵽ʱ�������־��1
        clock100ms = 0;
    }
    // 0.5������ʱ������
    if (++clock500ms>=V_T500ms)
    {
        clock500ms_flag = 1; //��0.5�뵽ʱ�������־��1
        clock500ms = 0;
    }

    // ˢ��ȫ������ܺ�LEDָʾ��
    TM1638_RefreshDIGIandLED(digit,pnt,led);

    // ��鵱ǰ�������룬0�����޼�������1-16��ʾ�ж�Ӧ����
    //   ������ʾ����λ�������
    key_code=TM1638_Readkeyboard();
    //digit[6]=key_code%10;
    //digit[5]=key_code/10;
    switch(key_state){
    case 0:
        if(key_code>0)
        {key_state=1;key_flag=1;}
        break;
    case 1:
        if(key_code==0)
        {key_state=0;key_timer=0;}
        else{
            key_timer++;
        }
        if(key_timer>=50&&key_timer%10==0)
            key_flag=1;
        break;
    default:
        key_state=0;
        break;
    }

}

//////////////////////////////
//         ������           //
//////////////////////////////

int main(void)
{
    unsigned char i=0,temp;
    Init_Devices( );
    while (clock100ms<3);   // ��ʱ60ms�ȴ�TM1638�ϵ����
    init_TM1638();      //��ʼ��TM1638

    gain_control();
    while(1)
    {
        /*
        if (clock100ms_flag==1)   // ���0.1�붨ʱ�Ƿ�
        {
            clock100ms_flag=0;
            // ÿ0.1���ۼӼ�ʱֵ�����������ʮ������ʾ���м�����ʱ��ͣ��ʱ
            if (key_code==0)
            {
                if (++test_counter>=10000) test_counter=0;
                digit[0] = test_counter/1000;       //�����λ��
                digit[1] = test_counter/100%10;     //����ʮλ��
                digit[2] = test_counter/10%10;      //�����λ��
                digit[3] = test_counter%10;         //����ٷ�λ��
            }
        }

        if (clock500ms_flag==1)   // ���0.5�붨ʱ�Ƿ�
        {
            clock500ms_flag=0;
            // 8��ָʾ��������Ʒ�ʽ��ÿ0.5�����ң�ѭ�����ƶ�һ��
            temp=led[0];
            for (i=0;i<7;i++) led[i]=led[i+1];
            led[7]=temp;
        }
        */
        if (key_flag==1){
            key_flag=0;
            switch (key_code)
            {
            case 1:
                if(++gain_state>GAIN_STATENUM) gain_state=1;
                gain_control();
                break;
            case 2:
                if(--gain_state==0) gain_state=15;
                gain_control();
                break;
            default:
                break;
            }
        }
        digit[2] = gain_state/10;
        digit[3] = gain_state%10;
    }
}
