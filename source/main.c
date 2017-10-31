#include <msp430.h> //MSP430�p�̃w�b�_�t�@�C��
#include "okokok.h" //���t�@�C���p�̃w�b�_�t�@�C��

//�^�C�}�[��`( 1MHz(�N���b�N���g��) / 8kHz(�T���v�����O���g��) = 125 , �^�C�}�[�J�E���^���@125-1 )
#define DEF_TIMER 124

/*** wav�p�J�E���^��`�@***/
static unsigned short wav_counter = WAV_ARR_NUM;

/*** �ݒ�p�֐��@***/
void init_config(void){

    WDTCTL = WDTPW | WDTHOLD;   // �E�H�b�`�h�b�N�^�C�}OFF

    /*** GPIO�ݒ� ***/
    P1OUT = BIT3;       // P1.3�̂� HIGH �ɂ���.
    P1DIR = 0x00;       // Port1.0 �` Port1.7 �� GPIO ���͂ɂ���.
    P1REN = BIT3;       // P1.3�v���A�b�v��R��L��.

    P1IES = BIT3;       // P1.3�� HIgh �� LOW �̂Ƃ��Ɋ��荞�ݏ������s��.
    P1IFG = 0x00;       // Port1.0 �` Port1.7 �̊��荞�݃t���O���O��.
    P1IE = BIT3;        // Port1.3 �̊��荞�݂�L���ɂ���.

    P2OUT = 0x00;               // P2.0 �` P2.7 �̏o�͂� LOW �ɂ���.
    P2SEL &= ~(BIT6 | BIT7);    // P2.6, P2.7 �� �|�[�g��GPIO�ɑI������.
    P2DIR = 0xFF;               // P2.0 �` P2.7 �� GPIO �o�͂ɂ���.
    P2REN = 0x00;               // Port2.0 �` Port2.7 �̃v���A�b�v/�v���_�E����R�𖳌�.
    P2IES = 0x00;               // Port2.0 �` Port2.7 �̊��荞�ݕ�����(�ǂ���ł�)
    P2IFG = 0x00;               // Port2.0 �` Port2.7 �̊��荞�݂͍s��Ȃ�.

    /*** P3.0 ������ (�|�[�g�͂Ȃ��Ă��O�̂���)***/
    P3OUT = 0x00;
    P3SEL = 0x00;
    P3DIR = 0x00;
    P3REN = 0x00;

    /*** �N���b�N�ݒ� ***/

    /*** CLOCK2 1MHz�ݒ� ***/
    BCSCTL2 = SELM_0 | DIVM_0 | DIVS_0;
    DCOCTL = 0x00;
    BCSCTL1 = CALBC1_1MHZ;      /* Set DCO to 1MHz */
    DCOCTL = CALDCO_1MHZ;

    /*** XT2OFF 1�{ ***/
    BCSCTL1 |= XT2OFF | DIVA_0;
    BCSCTL3 = XT2S_0 | LFXT1S_2 | XCAP_1;

    /*** �N���b�N�����肷��܂ő҂� ***/
    do{
        IFG1 &= ~OFIFG;     // Clear OSC fault flag
        __delay_cycles(50); // 50us delay
    }while (IFG1 & OFIFG);

    IFG2 &= ~(UCA0TXIFG | UCA0RXIFG);
    IE2 |= UCA0TXIE | UCA0RXIE;

    __bis_SR_register(GIE);

    /*** �N���b�N�̐ݒ�I�� ***/

    /*** �^�C�}�[�ݒ� ***/
    // �A�b�v���[�h�^�C�} : TA0CCR0 �܂ŃJ�E���g�A�b�v���ă^�C�}�[���荞�݂�����
    TA0CCTL0 = CM_0 | CCIS_0 | OUTMOD_0 | CCIE; // (�O�̓L���v�`�����[�h���g�p�ݒ�),�^�C�}���荞�ݗL��
    TA0CCR0 = DEF_TIMER;                        // �� �^�C�}�J�E���^���̐ݒ�
    TA0CTL = TASSEL_2 | ID_0 | MC_1;            // �^�C�}���� SMCLK ����,�N���b�N����1,�A�b�v���[�h�^�C�}
}

void main(void)
{
    init_config();  //�y���t�F�����ݒ�̌Ăяo��

    /*** P1.3 ���荞�݊J�n ***/
    P1IE |= BIT3;
    P1IFG &= ~BIT3;

    /*** TIMER0 ���荞�ݒ�~ ***/
    TA0CCTL0 &= ~CCIE;
    TA0CCR0 = 0;

    /*** �������[�v ***/
    while(1){
        /*** ���p�̃J�E���^�𐔂��Ă���Œ��͈ȉ��̏����͖�������. ***/
        if( WAV_ARR_NUM == wav_counter ){
            /*** P1.3�̃{�^�����������܂�CPU��~ ***/
            __bis_SR_register(CPUOFF + GIE); // CPU��~ + ���荞�ݔ���
        }
    }

}

/*** P1.3 �̃{�^�����������ƈȉ��̏���������. ***/
#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR_HOOK(void){

    wav_counter = 0;    //�J�E���^������

    /*** TIMER0 ���荞�݊J�n+�^�C�}�Z�b�g ***/
    TA0CCTL0 |= CCIE;
    TA0CCR0 = DEF_TIMER;

    /*** CPU�N�� ***/
    __bic_SR_register_on_exit(CPUOFF); // CPU ��ڊo�߂�����.
}

/*** �^�C�}�[�J�E���^���uDEF_TIMER�v�ɂȂ�����ȉ��̏���������. ***/
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR_HOOK(void){
    unsigned char temp_arr;

    /*** P1.3 ���荞�ݒ�~ ***/
    P1IE &= ~BIT3;
    P1IFG |= BIT3;

    /*** ���p�J�E���^���f�[�^���ȏ�̂Ƃ��@***/
    if( WAV_ARR_NUM <= wav_counter ){
        P2OUT = 0;  //P2OUT�����ׂ� LOW
        wav_counter = WAV_ARR_NUM;

        /*** TIMER0 ���荞�ݒ�~ ***/
        TA0CCTL0 &= ~CCIE;

        /*** P1.3 ���荞�݊J�n ***/
        P1IE |= BIT3;
        P1IFG &= ~BIT3;

    }else{
        /*** ���p�J�E���^���f�[�^�������̂Ƃ� ***/
        temp_arr = (unsigned char)(wav_arr[wav_counter]);
        P2OUT = temp_arr;   // �f�[�^�����̂܂� P2OUT �ɂ���邾���I
        wav_counter++;      // ���̔z�񐔂̃f�[�^�Ɉړ�����.
    }
}
