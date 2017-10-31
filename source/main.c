#include <msp430.h> //MSP430用のヘッダファイル
#include "okokok.h" //音ファイル用のヘッダファイル

//タイマー定義( 1MHz(クロック周波数) / 8kHz(サンプリング周波数) = 125 , タイマーカウンタ数　125-1 )
#define DEF_TIMER 124

/*** wav用カウンタ定義　***/
static unsigned short wav_counter = WAV_ARR_NUM;

/*** 設定用関数　***/
void init_config(void){

    WDTCTL = WDTPW | WDTHOLD;   // ウォッチドックタイマOFF

    /*** GPIO設定 ***/
    P1OUT = BIT3;       // P1.3のみ HIGH にする.
    P1DIR = 0x00;       // Port1.0 ～ Port1.7 を GPIO 入力にする.
    P1REN = BIT3;       // P1.3プルアップ抵抗を有効.

    P1IES = BIT3;       // P1.3を HIgh → LOW のときに割り込み処理を行う.
    P1IFG = 0x00;       // Port1.0 ～ Port1.7 の割り込みフラグを外す.
    P1IE = BIT3;        // Port1.3 の割り込みを有効にする.

    P2OUT = 0x00;               // P2.0 ～ P2.7 の出力を LOW にする.
    P2SEL &= ~(BIT6 | BIT7);    // P2.6, P2.7 の ポートをGPIOに選択する.
    P2DIR = 0xFF;               // P2.0 ～ P2.7 を GPIO 出力にする.
    P2REN = 0x00;               // Port2.0 ～ Port2.7 のプルアップ/プルダウン抵抗を無効.
    P2IES = 0x00;               // Port2.0 ～ Port2.7 の割り込み方向は(どちらでも)
    P2IFG = 0x00;               // Port2.0 ～ Port2.7 の割り込みは行わない.

    /*** P3.0 初期化 (ポートはなくても念のため)***/
    P3OUT = 0x00;
    P3SEL = 0x00;
    P3DIR = 0x00;
    P3REN = 0x00;

    /*** クロック設定 ***/

    /*** CLOCK2 1MHz設定 ***/
    BCSCTL2 = SELM_0 | DIVM_0 | DIVS_0;
    DCOCTL = 0x00;
    BCSCTL1 = CALBC1_1MHZ;      /* Set DCO to 1MHz */
    DCOCTL = CALDCO_1MHZ;

    /*** XT2OFF 1倍 ***/
    BCSCTL1 |= XT2OFF | DIVA_0;
    BCSCTL3 = XT2S_0 | LFXT1S_2 | XCAP_1;

    /*** クロックが安定するまで待つ ***/
    do{
        IFG1 &= ~OFIFG;     // Clear OSC fault flag
        __delay_cycles(50); // 50us delay
    }while (IFG1 & OFIFG);

    IFG2 &= ~(UCA0TXIFG | UCA0RXIFG);
    IE2 |= UCA0TXIE | UCA0RXIE;

    __bis_SR_register(GIE);

    /*** クロックの設定終了 ***/

    /*** タイマー設定 ***/
    // アップモードタイマ : TA0CCR0 までカウントアップしてタイマー割り込みを入れる
    TA0CCTL0 = CM_0 | CCIS_0 | OUTMOD_0 | CCIE; // (三つはキャプチャモード未使用設定),タイマ割り込み有効
    TA0CCR0 = DEF_TIMER;                        // ← タイマカウンタ数の設定
    TA0CTL = TASSEL_2 | ID_0 | MC_1;            // タイマ源は SMCLK から,クロック分周1,アップモードタイマ
}

void main(void)
{
    init_config();  //ペリフェラル設定の呼び出し

    /*** P1.3 割り込み開始 ***/
    P1IE |= BIT3;
    P1IFG &= ~BIT3;

    /*** TIMER0 割り込み停止 ***/
    TA0CCTL0 &= ~CCIE;
    TA0CCR0 = 0;

    /*** 無限ループ ***/
    while(1){
        /*** 音用のカウンタを数えている最中は以下の処理は無視する. ***/
        if( WAV_ARR_NUM == wav_counter ){
            /*** P1.3のボタンが押されるまでCPU停止 ***/
            __bis_SR_register(CPUOFF + GIE); // CPU停止 + 割り込み発動
        }
    }

}

/*** P1.3 のボタンが押されると以下の処理が発動. ***/
#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR_HOOK(void){

    wav_counter = 0;    //カウンタ初期化

    /*** TIMER0 割り込み開始+タイマセット ***/
    TA0CCTL0 |= CCIE;
    TA0CCR0 = DEF_TIMER;

    /*** CPU起動 ***/
    __bic_SR_register_on_exit(CPUOFF); // CPU を目覚めさせる.
}

/*** タイマーカウンタが「DEF_TIMER」になったら以下の処理が発動. ***/
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR_HOOK(void){
    unsigned char temp_arr;

    /*** P1.3 割り込み停止 ***/
    P1IE &= ~BIT3;
    P1IFG |= BIT3;

    /*** 音用カウンタがデータ数以上のとき　***/
    if( WAV_ARR_NUM <= wav_counter ){
        P2OUT = 0;  //P2OUTをすべて LOW
        wav_counter = WAV_ARR_NUM;

        /*** TIMER0 割り込み停止 ***/
        TA0CCTL0 &= ~CCIE;

        /*** P1.3 割り込み開始 ***/
        P1IE |= BIT3;
        P1IFG &= ~BIT3;

    }else{
        /*** 音用カウンタがデータ数未満のとき ***/
        temp_arr = (unsigned char)(wav_arr[wav_counter]);
        P2OUT = temp_arr;   // データをそのまま P2OUT にいれるだけ！
        wav_counter++;      // 次の配列数のデータに移動する.
    }
}
