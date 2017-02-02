# OpenBlocksファームウェア作成ガイド

ぷらっとホーム株式会社

Copyright (c) 2013-2017 Plat'Home CO., LTD.

# 1. はじめに

このソフトウェアを使えばOpenBlocksのファームウェアを作成することができます。内容は、シェルスクリプトとファームウェアに含めるファイルです。

# 2. 対応ファームウェア

OpenBlocksのファームウェアは、機種(Aファミリ, IoTファミリ, 600)とOS(Debian GNU/Linux)によって異なります。対応しているファームウェアは下の表をご覧ください。

機種|Debian|TARGET|DIST|ARCH
---|---|---|---|---
AX3|8|obsax3|jessie|armhf
AX3|7|obsax3|wheezy|armhf
A7|8|obsa7|jessie|armel
A7|7|obsa7|wheezy|armel
A6|7|obsa6|wheezy|armel
600|7|obs600|wheezy|powerpc
BXn|7|obsbx1|wheezy|i386
EX1|7|obsbx1|wheezy|i386

BXn: BX0, BX1, BX3

ファームウェアを作成するホストのOSはDebian GNU/Linux 8/amd64です。

項目TARGETとDISTは、ファームウェアを作成するときに実行するシェルスクリプトに指定する文字列です。後節で参照します。

# 3. 準備

## 3.1. 作成ホスト

作成ホストのOSはDebian 8/amd64です。ホストを用意して、そのOSをインストールします。ホストは物理マシンと仮想マシンのどちらでもよろしいです。インターネットに接続できるよう、ネッワークの設定をしてください。

## 3.2. Gitリポジトリの取得

このソフトウェアはGitリポジトリから取得できます。

作成ホストにgitパッケージをインストールします。

```
# apt-get update
# apt-get install git
```

Gitリポジトリを取得します。

```
$ git clone git://github.com/plathome/debian_based_firmware.git
```

以下では、ディレクトリdebian_based_firmwareに移動したものとして説明します。

## 3.3. クロス開発環境

ファームウェアの作成に必要なパッケージをインストールします。

```
# cd build_ramdisk
# ./build_crossenv.sh
```

## 3.4. カーネルソース

### 3.4.1. 取得

カーネルソースをぷらっとホームのFTPサイトftp.plathome.co.jpからダウンロードします。ファイルbuild_ramdisk/config.shのcase文中のKERNELとPATCHLEVELからカーネルソースのFTPサイトに置いてある場所が分ります。

以下は場所の例です。

* TARGET=obsax3, DIST=wheezy, KERNEL=3.2.40, PATCHLEVEL=4の場合  
http://ftp.plathome.co.jp/pub/OBSAX3/wheezy/3.2.40-4/linux-3.2.40-20140220.tar.gz
* TARGET=obsa7, DIST=wheezy, KERNEL=3.2.40, PATCHLEVEL=4の場合  
http://ftp.plathome.co.jp/pub/OBSA7/wheezy/3.2.40-4/linux-3.2.40-20140220.tar.gz
* TARGET=obsa6, DIST=wheezy, KERNEL=3.2.40, PATCHLEVEL=3の場合  
http://ftp.plathome.co.jp/pub/OBSA6/wheezy/3.2.40-3/linux-3.2.40-20140220.tar.gz
* TARGET=obs600, DIST=wheezy, KERNEL=3.10.25, PATCHLEVEL=0の場合  
http://ftp.plathome.co.jp/pub/OBS600/debian/files/wheezy/3.10.25-0/linux-3.10.25-obs600.tar.gz
* TARGET=obsbx1, DIST=wheezy, KERNEL=3.10.17, PATCHLEVEL=15の場合  
http://ftp.plathome.co.jp/pub/BX1/wheezy/3.10.17-15/linux-3.10.17-20160309.tar.xz

### 3.4.2. 展開

ディレクトリsourceには、TARGETをその名前としたディレクトリがあるので、そこにカーネルソースを展開してください。その結果、ディレクトリsource/TARGET/linux-KERNEL(およびlinux-KERNEL.orig)が作成されます。TARGETとKERNELは、ご自身の文字列に置き換えてください。

## 3.5. DVDイメージ

DIST=wheezyの場合、DebianのDVDイメージをDebianのミラーサイトからダウンロードしてディレクトリisofilesにコピーします。

ファイルbuild_ramdisk/config.shのcase文中のISOFILEからDVDイメージのファイル名が分ります。

以下はファイル名の例です。

* TARGET=obsax3の場合  
debian-7.7.0-armhf-DVD-1.iso
* TARGET=obsa7, obsa6の場合  
debian-7.7.0-armel-DVD-1.iso
* TARGET=obs600の場合  
debian-7.4.0-powerpc-DVD-1.iso
* TARGET=obsbx1の場合  
debian-7.8.0-i386-DVD-1.iso

これらのファイルは  
http://ftp.plathome.co.jp/pub/cdimages/debian/  
からも取得できます。

# 4. 作成

ファームウェアを作成します。シェルスクリプトbuild_ramdisk/buildall.shのオプション-MにTARGET、-DにDISTを指定します。TARGET=obsax3、DIST=wheezyの場合、

```
# cd build_ramdisk
# ./buildall.sh -M obsax3 -D wheezy
```

と実行します。

作成されたファームウェア一式は、ディレクトリrelease/TARGET/DIST/KERNEL-PATCHLEVELに置かれます。TARGET, DIST,KERNEL, PATCHLEVELは、ご自身の文字列に置き換えてください。

例えば、ディレクトリrelease/obsax3/wheezy/3.2.40-3には、以下のファイルが作成されます。

* MD5.obsax3  
MD5チェックサムファイル
* System.map  
カーネルのシステムマップファイル
* kernel-image-3.2.40-3.deb  
コマンドdpkgなどでインストールするためのパッケージ
* ramdisk-wheezy.obsax3.img.lzma  
ファームウェアのRAMディスクイメージ
* uImage.initrd.obsax3  
コマンドflashcfgでインストールするためのファームウェア
* zImage.gz  
ファームウェアのカーネルイメージ

# 5. カスタマイズ

本節ではファームウェアのカスタマイズの手順を説明します。

シェルスクリプトbuild_ramdisk/buildall.shを実行することは、DIST=wheezy, TARGET=obsax3の場合、以下の通り実行することと同じです。

```
# DIST=wheezy TARGET=obsax3 ./build_debootstrap.sh
# DIST=wheezy TARGET=obsax3 ./build_kernel.sh
# DIST=wheezy TARGET=obsax3 ./build_ramdisk.sh
# DIST=wheezy TARGET=obsax3 ./release_firmware.sh
```

TARGETとDISTについては、予め次のように設定することで、毎回の指定は不要になります。

```
# export DIST=wheezy TARGET=obsax3
```

内容の詳細は各シェルスクリプトを読んでいただくことにして、簡単に説明しておきます。以下のTARGETとDISTは、ご自身の文字列に置き換えてください。

* build_debootstrap.sh  
debootstrapを実行し、ディレクトリrootfs/DIST_TARGET以下にDebianを仮インストールしています。
* build_kernel.sh  
カーネルとカーネルモジュールのコンパイルをしています。
* build_ramdisk.sh  
ディレクトリrootfs/DIST_TARGET以下に仮インストールしたDebianをOpenBlocks用に変更して、RAMディスクイメージを作成しています。
* release_firmware.sh  
ファームウェア一式を作成しています。

上記のシェルスクリプトを手動で実行すれば、その途中で、カーネルとRAMディスクイメージのカスタマイズができます。

ファイルbuild_ramdisk/config.shのcase文中のPATCHLEVELには文字列などを追加、例えば3customとすることをお勧めします。公式ファームウェアとの区別がはっきりします。

カスタマイズの前に、その準備として、シェルスクリプトbuild_ramdisk/buildall.shを実行しておいてください。

## 5.1. カーネル

カーネルのコンフィグレーションを変更してカーネルをカスタマイズすることができます。カーネルをカスタマイズしない場合には、「5.2. RAMディスクイメージ」節に進んでください。

### 5.1.1. コンフィグレーションの変更

次のコマンドを実行して、カーネルのコンフィグレーションを変更します。TARGETとDISTは、ご自身の文字列に置き換えてください。

```
# DIST=wheezy TARGET=obsax3 ./kernel_menuconfig.sh
```

変更したコンフィグレーションファイルはsource/TARGET/linux-KERNEL.dot.configにコピーされます。TARGETとKERNELは、ご自身の文字列に置き換えてください。

### 5.1.2. コンパイル

シェルスクリプトbuild_ramdisk/build_kernel.shから順番に実行するか、シェルスクリプトbuild_ramdisk/buildall.shを実行してください。RAMディスクイメージをカスタマイズする場合、シェルスクリプトbuild_ramdisk/build_kernel.shを実行して、次節に進んでください。

## 5.2. RAMディスクイメージ

シェルスクリプトbuild_ramdisk/build_ramdisk.shの実行が終了すると、ディレクトリrootfs/DIST_TARGETに、RAMディスクに含まれるファイルが仮インストールされます。ディレクトリrootfs/DIST_TARGET以下のファイルを変更したり、パッケージを追加すれば、RAMディスクイメージのカスタマイズができます。DISTとTARGETは、ご自身のものに置き換えてください。

### 5.2.1. パッケージの取得

追加するパッケージをDebianのミラーサイトからダウンロードして、ディレクトリextradebs/DISTにコピーします。DISTは、ご自身の文字列に置き換えてください。

### 5.2.2. パッケージの追加

次のコマンドを実行してパッケージを追加します。DIST=wheezy,TARGET=obsax3の場合、

```
# DIST=wheezy TARGET=obsax3 ./08_add_extra_packages.sh
```

と実行します。

パッケージが追加されたかは、ファイルrootfs/DIST_TARGET/var/lib/dpkg/statusで確認できます。TARGETとDISTは、ご自身の文字列に置き換えてください。

### 5.2.3. RAMディスクイメージの作成

ディレクトリrootfs/DIST_TARGET以下のファイルをもとにRAMディスクイメージを作成します。TARGETとDISTは、ご自身の文字列に置き換えてください。DIST=wheezy, TARGET=obsax3の場合、

```
# DIST=wheezy TARGET=obsax3 ./99_create_ramdisk.sh
```

と実行します。

ファームウェアを作成します。DIST=wheezy, TARGET=obsax3の場合、

```
# DIST=wheezy TARGET=obsax3 ./release_firmware.sh
```

と実行します。

## 5.3. まとめ

一度「5.1.1. コンフィグレーションの変更」と「5.2.1. パッケージの取得」をすませておけば、シェルスクリプトbuild_ramdisk/buildall.shを実行するだけでカスタマイズのすんだファームウェアが作成できます。

# 6. ファームウェアの更新

ファームウェアパッケージkernel-image-KERNEL-PATCHLEVEL-TARGET.debをOpenBlocksにネットワーク経由もしくはUSBメモリでコピーしてください。KERNEL，PATCHLEVELとTARGETは、ご自身の文字列に置き換えてください。

次のコマンド

```
# dpkg -i kernel-image-KERNEL-PATCHLEVEL-TARGET.deb
```

を実行してください。

RAMディスクモードの場合はflashcfgコマンドを実行することをお勧めします。ファームウェアのバージョンとパッケージに関する情報内のバージョンが合わなくなる場合があります。（合わなくても動作に影響はありません。）

```
# flashcfg -S
```

再起動すれば、ファームウェアの更新は終了です。
