# OpenBlocksファームウェア作成ガイド

ぷらっとホーム株式会社

Copyright (c) 2013-2024 Plat'Home CO., LTD.

# 1. はじめに

このソフトウェアを使えば OpenBlocks のファームウェアを作成することができます。内容は、シェルスクリプトとファームウェアに含めるファイルです。

# 2. 対応ファームウェア

OpenBlocks のファームウェアは、機種 (OpenBlocks シリーズ, OpenBlocks IoT シリーズ) と OS (Debian GNU/Linux) によって異なります。対応しているファームウェアは下の表をご覧ください。

機種|Debian|TARGET|DIST|ARCH|ファームウェアを作成するホストのOS
---|---|---|---|---|---
IX9|11|obsix9|bullseye|amd64|Debian GNU/Linux 11/amd64
IX9|10|obsix9|buster|amd64|Debian GNU/Linux 10/amd64
IX9|12|obsix9r|bookworm|amd64|Debian GNU/Linux 12/amd64
IX9|11|obsix9r|bullseye|amd64|Debian GNU/Linux 11/amd64
IX9|10|obsix9r|buster|amd64|Debian GNU/Linux 10/amd64
A16|12|obsa16|bookworm|arm64|Debian GNU/Linux 12/amd64
A16|11|obsa16|bullseye|arm64|Debian GNU/Linux 11/amd64
AX3|9|obsax3|stretch|armhf|Debian GNU/Linux 9/amd64
AX3|8|obsax3|jessie|armhf|Debian GNU/Linux 8/amd64
AX3|7|obsax3|wheezy|armhf|Debian GNU/Linux 8/amd64
A7|9|obsa7|stretch|armel|Debian GNU/Linux 9/amd64
A7|8|obsa7|jessie|armel|Debian GNU/Linux 8/amd64
A7|7|obsa7|wheezy|armel|Debian GNU/Linux 8/amd64
A6|7|obsa6|wheezy|armel|Debian GNU/Linux 8/amd64
600|7|obs600|wheezy|powerpc|Debian GNU/Linux 8/amd64
BXn|11|obsbx1s|bullseye|i386|Debian GNU/Linux 11/amd64
BXn|10|obsbx1s|buster|i386|Debian GNU/Linux 10/amd64
BXn|9|obsbx1s|stretch|i386|Debian GNU/Linux 9/amd64
BXn|8|obsbx1|jessie|i386|Debian GNU/Linux 8/amd64
BXn|7|obsbx1|wheezy|i386|Debian GNU/Linux 8/amd64
EX1|11|obsbx1s|bullseye|i386|Debian GNU/Linux 11/amd64
EX1|10|obsbx1s|buster|i386|Debian GNU/Linux 10/amd64
EX1|9|obsbx1s|stretch|i386|Debian GNU/Linux 9/amd64
EX1|8|obsbx1|jessie|i386|Debian GNU/Linux 8/amd64
EX1|7|obsbx1|wheezy|i386|Debian GNU/Linux 8/amd64
FX1|11|obsfx1|bullseye|arm64|Debian GNU/Linux 11/amd64
DX1|11|obsfx0|bullseye|arm64|Debian GNU/Linux 11/amd64
VX2|11|obsvx2|bullseye|amd64|Debian GNU/Linux 11/amd64
VX2|10|obsvx2|buster|amd64|Debian GNU/Linux 10/amd64
VX2|9|obsvx2|stretch|amd64|Debian GNU/Linux 9/amd64
VX1|8|obsvx1|jessie|amd64|Debian GNU/Linux 8/amd64

BXn: BX0, BX1, BX1S, BX3, BX5

項目 TARGET と DIST は、ファームウェアを作成するときに実行するシェルスクリプトに指定する文字列です。後節で参照します。

# 3. 準備

## 3.1. 作成ホスト

作成ホストの OS は、2. 対応ファームウェアの表を参照してください。ホストを用意して、その OS をインストールします。
ホストは物理マシンと仮想マシンのどちらでもよろしいです。インターネットに接続できるよう、ネッワークの設定をしてください。

## 3.2. Gitリポジトリの取得

このソフトウェアはGitリポジトリから取得できます。

作成ホストにgitパッケージをインストールします。

```
# apt update
# apt install git
```

Gitリポジトリを取得します。

```
$ git clone https://github.com/plathome/debian_based_firmware.git
```

以下では、ディレクトリ debian_based_firmware に移動したものとして説明します。

### 3.2.1. タグの指定

リリースしたファームウェア毎にタグを付けています。次のコマンドで、タグの一覧が表示されます。

```
$ git tag
```

ファームウェアのバージョンとタグの対応は、次のコマンドから知ることができます。

```
$ git show タグ名
```

タグ名を指定してファームウェアを作成するには、次のコマンドを実行します。

```
$ git checkout refs/tags/タグ名
```

## 3.3. クロス開発環境

ファームウェアの作成に必要なパッケージをインストールします。

* 作成するホストの OS が、Debian GNU/Linux 8/amd64 の場合。

```
# cd build_ramdisk
# ./build_crossenv.sh
```

* 作成するホストの OS が、Debian GNU/Linux 9/amd64 の場合。

```
# cd build_ramdisk
# ./build_crossenv-stretch.sh
```

* 作成するホストの OS が、Debian GNU/Linux 10/amd64 の場合。

```
# cd build_ramdisk
# ./build_crossenv-buster.sh
```

* 作成するホストの OS が、Debian GNU/Linux 11/amd64 の場合。

```
# cd build_ramdisk
# ./build_crossenv-buster.sh
```

* 作成するホストの OS が、Debian GNU/Linux 12/amd64 の場合。

```
# cd build_ramdisk
# ./build_crossenv-bookworm.sh
```

## 3.4. カーネルソースファイル

### 3.4.1. 取得

カーネルソースファイルをぷらっとホームのFTPサイト ftp.plathome.co.jp からダウンロードしてディレクトリ source にコピーします。

ファイル build_ramdisk/config.sh の case 文中の KERNEL と PATCHLEVEL からカーネルソースファイルの FTP サイトに置いてある場所が分ります。

以下は場所の例です。

* TARGET=obsix9r, DIST=bookworm, KERNEL=6.1.92, PATCHLEVEL=1 の場合</br>
https://ftp.plathome.co.jp/pub/OBSIX9R/bookworm/6.1.92-1/linux-6.1.92_obsix9r_20240716.tar.xz

### 3.4.2. 展開

ディレクトリ source には、TARGET をその名前としたディレクトリがあるので、そこにカーネルソースファイルを展開してください。その結果、
ディレクトリ source/TARGET/linux-KERNEL(およびlinux-KERNEL.orig) が作成されます。

```
# rm -rf source/TARGET/linux-KERNEL (存在するならば削除します。)
# rm -rf source/TARGET/linux-KERNEL.orig (存在するならば削除します。)
# tar xpf source/<カーネルソースファイル名> -C source/TARGET/
```

## 3.5. DVDイメージ

Debian の DVD イメージを Debian のミラーサイトからダウンロードしてディレクトリ isofiles にコピーします。

ファイル build_ramdisk/config.sh の case 文中の ISOFILE から DVD イメージのファイル名が分ります。

以下はファイル名の例です。

* TARGET=obsix9r, DIST=bookworm の場合</br>
debian-12.0.0-amd64-DVD-1.iso

これらのファイルは</br>
[https://ftp.plathome.co.jp/pub/cdimages/debian/](https://ftp.plathome.co.jp/pub/cdimages/debian/)</br>
[https://cdimage.debian.org/mirror/cdimage/archive/](https://cdimage.debian.org/mirror/cdimage/archive/)</br>
からも取得できます。

# 4. 作成

ファームウェアを作成します。シェルスクリプト build_ramdisk/buildall.sh のオプション -M に TARGET、-D に DIST を指定します。 
TARGET=obsix9r、DIST=bookworm の場合、次のように実行します。

```
# cd build_ramdisk
# ./buildall.sh -M obsix9r -D bookworm
```

作成されたファームウェア一式は、ディレクトリ release/TARGET/DIST/KERNEL-PATCHLEVEL に置かれます。

# 5. カスタマイズ

本節ではファームウェアのカスタマイズの手順を説明します。

シェルスクリプト build_ramdisk/buildall.sh を実行することは、DIST=bookworm, TARGET=obsix9r の場合、以下の通り実行することと同じです。

```
# DIST=bookworm TARGET=obsix9r ./build_debootstrap.sh
# DIST=bookworm TARGET=obsix9r ./build_kernel.sh
# DIST=bookworm TARGET=obsix9r ./build_ramdisk.sh
# DIST=bookworm TARGET=obsix9r ./release_firmware.sh
```

TARGET と DIST については、予め次のように設定することで、毎回の指定は不要になります。

```
# export DIST=bookworm TARGET=obsix9r
```

内容の詳細は各シェルスクリプトを読んでいただくことにして、簡単に説明しておきます。

* build_debootstrap.sh  
debootstrap を実行し、ディレクトリ rootfs/DIST_TARGET 以下に Debian を仮インストールしています。
* build_kernel.sh  
カーネルとカーネルモジュールのコンパイルをしています。
* build_ramdisk.sh  
ディレクトリ rootfs/DIST_TARGET 以下に仮インストールした Debian を OpenBlocks 用に変更して、RAM ディスクイメージを作成しています。
* release_firmware.sh  
ファームウェア一式を作成しています。

上記のシェルスクリプトを手動で実行すれば、その途中で、カーネルと RAM ディスクイメージのカスタマイズができます。

ファイル build_ramdisk/config.sh の case 文中の PATCHLEVEL には文字列などを追加、例えば 3custom とすることをお勧めします。公式ファームウェアとの区別がはっきりします。

カスタマイズの前に、その準備として、シェルスクリプト build_ramdisk/buildall.sh を実行しておいてください。

## 5.1. カーネル

カーネルのコンフィグレーションを変更してカーネルをカスタマイズすることができます。カーネルをカスタマイズしない場合には、「5.2. RAMディスクイメージ」節に進んでください。

### 5.1.1. コンフィグレーションの変更

次のコマンドを実行して、カーネルのコンフィグレーションを変更します。

```
# DIST=bookworm TARGET=obsix9r ./kernel_menuconfig.sh
```

変更したコンフィグレーションファイルは source/TARGET/linux-KERNEL.dot.config にコピーされます。

### 5.1.2. コンパイル

シェルスクリプト build_ramdisk/build_kernel.sh から順番に実行するか、シェルスクリプト build_ramdisk/buildall.sh を実行してください。RAMディスクイメージをカスタマイズする場合、
シェルスクリプトb uild_ramdisk/build_kernel.sh を実行して、次節に進んでください。

## 5.2. RAMディスクイメージ

シェルスクリプトbuild_ramdisk/build_ramdisk.sh の実行が終了すると、ディレクトリ rootfs/DIST_TARGET に、RAM ディスクに含まれるファイルが仮インストールされます。
ディレクトリrootfs/DIST_TARGET 以下のファイルを変更したり、パッケージを追加すれば、RAMディスクイメージのカスタマイズができます。

### 5.2.1. パッケージの取得

追加するパッケージをDebianのミラーサイトからダウンロードして、ディレクトリ extradebs/DIST にコピーします。

### 5.2.2. パッケージの追加

次のコマンドを実行してパッケージを追加します。DIST=bookworm,TARGET=obsix9rの場合、次のように実行します。

```
# DIST=bookworm TARGET=obsix9r ./06_add_extra_packages.sh
```


パッケージが追加されたかは、ファイル rootfs/DIST_TARGET/var/lib/dpkg/status で確認できます。

### 5.2.3. RAMディスクイメージの作成

ディレクトリ rootfs/DIST_TARGET 以下のファイルをもとに RAM ディスクイメージを作成します。DIST=bookworm, TARGET=obsix9r の場合は、次のように実行します。

```
# DIST=bookworm TARGET=obsix9r ./99_create_ramdisk.sh
```

ファームウェアを作成します。DIST=bookworm, TARGET=obsix9r の場合は、次のように実行します。

```
# DIST=bullseye TARGET=obsix9r ./release_firmware.sh
```

## 5.3. まとめ

一度「5.1.1. コンフィグレーションの変更」と「5.2.1. パッケージの取得」をすませておけば、シェルスクリプトbuild_ramdisk/buildall.shを実行するだけでカスタマイズのすんだファームウェアが作成できます。

# 6. ファームウェアの更新

ファームウェアパッケージkernel-image-KERNEL-PATCHLEVEL-TARGET.debを OpenBlocks にネットワーク経由もしくは USB メモリでコピーし、
次のコマンドを実行してパッケージをインストールして下さい。

```
# dpkg -i kernel-image-KERNEL-PATCHLEVEL-TARGET.deb
```

RAM ディスクモードの場合は flashcfg コマンドを実行することをお勧めします。ファームウェアのバージョンとパッケージに関する情報内のバージョンが合わなくなる場合があります。（合わなくても動作に影響はありません。）

```
# flashcfg -S
```

再起動すれば、ファームウェアの更新は終了です。
