"""
Read by python3.6 64bit

"""
import glob
import datetime as dt
import wave as wav

#例外処理
"""
モノラル限定
サンプルサイズ 8bit 限定
サンプルレート 8kHz限定
フレーム数 14000以下(約13KB)

"""
if __name__ == '__main__':

    #最大フレーム数定義＋エラーステイト初期化
    max_frame_size = 14000  #最大フレーム数
    errors_num = 0          #エラーステイト

    #ログ生成用ファイルオープン
    logs = open('make_file_log.txt','w')

    # ログの日時を入力
    display_today = dt.datetime.today()
    logs.write("日付と時間：" + display_today.strftime("%Y/%m/%d %H:%M:%S") + '\n' )

    #正常系
    try:

        #wavファイル読み込み
        file_path = glob.glob("./*.wav")

        #リストを数える(wavファイルの数を取得する)
        wav_list_counter = len(file_path)

        if (0 != wav_list_counter):
            #wavファイルを開く(最初に開いたもののみ適応)
            wf = wav.open(file_path[0],"rb")

            # ログファイルに読み込みファイル情報
            logs.write("------------------------------------------------------------------------" + '\n')
            logs.write("wavファイル名 : " + str(file_path[0]) + '\n')
            logs.write("チャネル数 : %d" % wf.getnchannels() + '\n')
            logs.write("サンプルサイズ : %d [bit]" % ( wf.getsampwidth() * 8 )  + '\n')
            logs.write("サンプリングレート : %d [Hz]" % (wf.getframerate())  + '\n')
            logs.write("フレーム数 : %d" % wf.getnframes()  + '\n')
            logs.write("再生時間 : %f [sec]" % (float(wf.getnframes()) / wf.getframerate() )  + '\n')
            logs.write("------------------------------------------------------------------------" + '\n')

            if(max_frame_size < wf.getnframes()):
                #フレーム数を超えた
                errors_num = 1
            elif(wf.getsampwidth() != 1):
                #サンプルサイズが異なる
                errors_num = 2
            elif(wf.getframerate() != 8000):
                #サンプリングレートが異なる
                errors_num = 3
            elif(wf.getnchannels() != 1):
                #チャネル数が異なる
                errors_num = 4
            else:
                #エラーなしということにしておく
                errors_num = 0
        else:
            logs.write("エラー：" + "wavファイルがありません")
            errors_num = 5

        #ファイル生成
        if(errors_num == 0):
            #フレーム数を取得
            frame_fig = wf.getnframes()
            wav_data = wf.readframes(frame_fig) #wavデータ取得
            write_counter = 0

            #ヘッダファイルオープン
            file = open('okokok.h','w')

            #書き込み開始
            file.write( "#ifndef OKOKOK_H_" + '\n')
            file.write( "#define OKOKOK_H_" + '\n' + '\n')
            file.write( "#define WAV_ARR_NUM %d" %(frame_fig) + '\n' + '\n')
            file.write( "const unsigned char wav_arr[WAV_ARR_NUM] = {" + '\n' + '\t')

            #wav内部書き込み(16データ毎に改行)
            while write_counter < (frame_fig - 1):
                file.writelines(str(wav_data[write_counter]))
                file.writelines(',')
                write_counter = write_counter + 1
                if (write_counter % 16) == 0 :
                    file.writelines('\n' + '\t')

            #最終データ処理
            file.write(str(wav_data[write_counter]) + '};' + '\n' + '\n')
            file.write("#endif /* OKOKOK_H_ */" + '\n')

            logs.write("ファイル作成できました。")

            wf.close()      #wavファイルクローズ
            file.close()    #wavデータ書き込みファイルクローズ

        elif(errors_num==1):
            # ログファイルにエラー内容を記述
            logs.write("エラー：" + "フレーム数を超えました。%d 以内にしてください。" %(max_frame_size) + '\n')

        elif(errors_num==2):
            # ログファイルにエラー内容を記述
            logs.write("エラー：" + "サンプルサイズは 8bit でお願いします。" + '\n')

        elif(errors_num==3):
            # ログファイルにエラー内容を記述
            logs.write("エラー：" + "サンプリングレートは 8000Hz でお願いします。" + '\n')

        elif(errors_num==4):
            # ログファイルにエラー内容を記述
            logs.write("エラー：" + "モノラル(チャネル数 = 1)でお願いします。" + '\n')
        else:
            pass
    except:
        logs.write("ファイル作成できませんでした。" + '\n')

        #予期せぬ終了の場合もクローズ
        wf.close()      #wavファイルクローズ
        file.close()    #wavデータ書き込みファイルクローズ

    finally:
        logs.close()    #logファイルクローズ
        
