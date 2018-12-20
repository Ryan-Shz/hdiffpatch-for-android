# hdiffpatch-for-android
基于差分算法库的android ndk使用示例，开源库地址:
> https://github.com/sisong/HDiffPatch

示例中只提供了合成功能，也就是客户端所需要的合成算法。而差分算法一般在服务器中执行。

如果需要测试，可以通过编译该作者的源码，在linux中通过命令行生成差分包。

示例中提供了两个apk测试，自行把old apk和差分包导入/sdcard/patch_test目录中，在输入框输入测试apk名称，然后点击apply patch按钮即可合成。

本示例主要为c库编译困难的同学提供示例。若要直接应用到项目中，请自行进行完整测试。 
