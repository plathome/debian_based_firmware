-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA256

Format: 1.0
Source: parted
Binary: parted, parted-udeb, libparted0, libparted0debian1, libparted0-udeb, libparted0-i18n, libparted0-dev, libparted0debian1-dbg, parted-doc
Architecture: any all
Version: 2.3-12
Maintainer: Parted Maintainer Team <parted-maintainers@lists.alioth.debian.org>
Uploaders: Otavio Salvador <otavio@debian.org>, Bastian Blank <waldi@debian.org>, Xavier Oswald <xoswald@debian.org>, Colin Watson <cjwatson@debian.org>
Homepage: http://www.gnu.org/software/parted
Standards-Version: 3.8.4
Vcs-Browser: http://git.debian.org/?p=parted/debian/parted.git
Vcs-Git: git://git.debian.org/git/parted/debian/parted.git
Build-Depends: dpkg-dev (>= 1.15.7~), debhelper (>= 8.1.3), quilt, libncurses-dev | libncurses5-dev, libreadline-dev | libreadline6-dev, libdevmapper-dev (>= 2:1.02.33) [linux-any], uuid-dev, gettext, texinfo (>= 4.2), debianutils (>= 1.13.1), libblkid-dev, po4a, pkg-config, check
Package-List: 
 libparted0 deb libs extra
 libparted0-dev deb libdevel optional
 libparted0-i18n deb localization optional
 libparted0-udeb udeb debian-installer extra
 libparted0debian1 deb libs optional
 libparted0debian1-dbg deb debug extra
 parted deb admin optional
 parted-doc deb doc optional
 parted-udeb udeb debian-installer extra
Checksums-Sha1: 
 9ef653c1f97c88b52d8e41f26ea3a8e3379f9cd6 2974168 parted_2.3.orig.tar.gz
 28451a35aab28bda0135b26be138d8184b12292e 87695 parted_2.3-12.diff.gz
Checksums-Sha256: 
 e81fa140805b5cd029ff6dda5cfa94d223e83ac182ebcae94f841d62ce468829 2974168 parted_2.3.orig.tar.gz
 c276e72bbde92d3d75ef2661ec31a0b026ab0da5ccefa53c82e09c3846a51ea5 87695 parted_2.3-12.diff.gz
Files: 
 30ceb6df7e8681891e865e2fe5a7903d 2974168 parted_2.3.orig.tar.gz
 a71f8b735a7cc49deaacfc46ec09d344 87695 parted_2.3-12.diff.gz

-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1.4.12 (GNU/Linux)
Comment: Colin Watson <cjwatson@debian.org> -- Debian developer

iQIVAwUBUOq/CTk1h9l9hlALAQhXxw//Q3/vKhEecE6a0+owMB1F0ke06FEWBgjY
G62Yv8C7NQtO7IXrsRZnVapOp/qt4shf1zbTQgddtY+SW05S14AMUiBiTeBDV+F0
1MrcDBgj8LhLiI7HlfKvBf6VLCere3OdZIlcXlXXEfLwfIZzEM3aS4laOb5RojiC
IyVWDvHmiDv34sb5P+QolWWebI3k5LyLuTw6w01Qh7SdyhvIUhLunkeFvUQPdr1A
eYPdO3V6i0W17DSN2Xs6t/PIn5ea38QtxV/KxmtarNQoOFzskr9OGnNbeGd5P4R2
ldR0c/lCVwHG6Zq6uK1cPZhkEK9iGT7pUsafXo2qmQi8jTeIew1/3Qr+5m7aGJJc
8/QJ5kooTzOQCxTOR2yiSK/0E5j0z6B9uctrydbmpjwBrJ6YShGG/FLE/rOaWLHr
Q+VjRk2IJxiSGXF47dWjZc4cxXMsamyjO5hu/p0R27pLpOQl0AZNShpwlpP9BIF+
AsVXt0YE9LGOz5yWs5Xly0MNgAlV6ywmgwlpX0R0VZDum1fOiFn2KnpMpdtpmYFY
oNDpqAs+TuVpSwCRtfCoKFmfswJ59YAS52ijYB0SEU4wsoSIB5z70xOn0Svv8VeK
PgUWwRmSOj6ytx74WXlbenqsIfGh8SzpTOl9MZRKBN/RxuDawb0w8oaqvffwyZWX
j1EY48MYer4=
=dTZ4
-----END PGP SIGNATURE-----
