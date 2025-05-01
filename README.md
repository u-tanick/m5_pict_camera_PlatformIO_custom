# m5_pict_camera_PlatformIO_custom

Yuikawa-Akiraさんが[こちらで公開されているPict_Cameraのコード](https://gist.github.com/Yuikawa-Akira)をもとに、PlatformIO化＋自分の手持ちの機器に合わせた改修＋機能追加を行ったものです。

オリジナルのピクセルアート風の画像を保存する機能に加えて、以下の機能を追加実装しています。

- スクロールユニットによるカメラシャッター機能
  - オリジナルはエンコーダーユニットですが、スクロールユニットに変更しています。
- デジタルズーム機能（1.0～4.0倍）
  - カメラ撮影モードの際に、スクロールユニットを右に回転させるとズームします。
  - ズームした状態でスクロールユニットを押すとズームされた状態の画像でピクセルアート画像が保存されます。
- プレビューモード機能
  - カメラモードで左にスクロールすると画面が「Press Button Previre Mode」と表示されるのでそこでスクロールユニットを押すと撮像した画像をLCDディスプレイでプレビューする機能に切り替わります。
  - プレビューモード時に再度スクロールユニットを押すとカメラモードに戻ります。
  - プレビューモードに切り替わる際は、その時点の画像のリストを作成するため数秒の処理待ちが発生します。
  - また、画像のIDは電源のON/OFFでリセットされるため必ずしも新しいもの順といった規則性のある順序にはなっていません。


以下の機器をセットする想定で作成したカメラケースのデータを、stlフォルダに格納しています。

- [AtomS3Rカメラ](https://www.switch-science.com/products/9916)
- [ATOMIC TFカードリーダー](https://www.switch-science.com/products/9423)
- [LCDディスプレイユニット](https://www.switch-science.com/products/7358)
- [スクロールユニット](https://www.switch-science.com/products/9917)
- [拡張ハブユニット](https://www.switch-science.com/products/5696)
- [GROVE互換ケーブル 10cm](https://www.switch-science.com/products/5213)
  - 1本
- [GROVE互換ケーブル 5cm](https://www.switch-science.com/products/8664)
  - 2本
- [SDカード Class10 16GB以下](https://www.yodobashi.com/?spcs=Specvaluecode_500000301004005003,500000301004005005_0001_0000000025_0000033030&spcs=Specvaluecode_500000301004005003,500000301004005005_0001_0000000338_0000009621&word=SD%E3%82%AB%E3%83%BC%E3%83%89+16GB)
- [レゴテクニック　ピン](https://www.amazon.co.jp/dp/B07B9R4J55/)
  - 1本
- [モバイルバッテリー：cheero canvas 3200mAh](https://www.amazon.co.jp/dp/B0BPC5BPVK/)
- [USB Type-Cケーブル 10cm以下](https://www.amazon.co.jp/dp/B0DJ85RLDN/)
  - 両端 Type-C のものはなぜか通電しなかったので、Type-A - Type-C を使用しました。
  - あまり10cm以下の商品は少ないのですが、購入の際はご注意（自己責任）ください。
- [100均で売っている 木ネジ（12mm*3mm）](https://jp.daisonet.com/products/4991203166913) 
