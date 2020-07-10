#!/bin/bash

# compile ipbl
#~ ipbl_DIR=""
#~ cd $ipbl_DIR
if [ ! -f "../bin/ipbl" ]; then
	echo "execute make before gen_package"
	exit 1
fi

PREFIX=$1
KERNEL_VERSION=`uname -r | cut -d. -f1-2`
DEBIAN_VERSION="stretch"

if [ $KERNEL_VERSION == "3.2" ]; then
	DEBIAN_VERSION="wheezy"
fi

if [ $KERNEL_VERSION == "3.16" ]; then
	DEBIAN_VERSION="jessie"
fi

# Get the ipbl version
VERSION=$(../bin/ipbl -V)
BIN_DIR=$PREFIX/bin
ETC_DIR=$PREFIX/etc/ipbl

# Create a package tmp directory
PACKAGE_DIR="../package_deb"
PACKAGE_DIR_NAME="package_deb"
mkdir -p $PACKAGE_DIR/DEBIAN

# Licence head
HEAD=""

# Create DEBIAN package files
touch $PACKAGE_DIR/DEBIAN/conffiles
echo "$HEAD" > $PACKAGE_DIR/DEBIAN/control
echo "#!/bin/bash

$HEAD

chmod 755 ${BIN_DIR}/ipbl
/etc/init.d/ipbl restart

exit 0;" > $PACKAGE_DIR/DEBIAN/postinst
echo "#!/bin/bash

$HEAD

if [ ! -f \"/usr/include/libpool.h\" ]; then
	cd /usr/share/doc/libdotconf-dev/examples/libpool/
	gzip -d pool.c.gz > /dev/null
	make > /dev/null
	cp libpool.a /usr/lib
	cp libpool.h /usr/include
fi
	
exit 0;" > $PACKAGE_DIR/DEBIAN/preinst

echo "Package: ipbl
Version: $VERSION
Maintainer: ZEVENET Company SL <support@zevenet.com>
Architecture: amd64
Section: admin
Priority: optional
Description: RBL client
 RBL client" > $PACKAGE_DIR/DEBIAN/control
 
 
 if [ "$DEBIAN_VERSION" == "wheezy" ]; then
	echo "Depends: libdotconf1.0, libldns1, libnetfilter-queue1, libnfnetlink0" >> $PACKAGE_DIR/DEBIAN/control
else
	if [ "$DEBIAN_VERSION" == "jessie" ]; then
		echo "Depends: libdotconf0, libldns1, libnetfilter-queue1, libnfnetlink0, libdotconf-dev" >> $PACKAGE_DIR/DEBIAN/control
	#  [ DEBIAN_VERSION == "stretch" ]
	else
		echo "Depends: libdotconf0, libldns2, libnetfilter-queue1, libnfnetlink0, libdotconf-dev" >> $PACKAGE_DIR/DEBIAN/control
	fi
fi


# permissions
chmod 555 $PACKAGE_DIR/DEBIAN/preinst
chmod 555 $PACKAGE_DIR/DEBIAN/postinst


mkdir -p $PACKAGE_DIR/etc/init.d

echo "#! /bin/sh

### BEGIN INIT INFO
# Provides:		ipbl
# Required-Start:	
# Required-Stop:	
# Default-Start:	2 3 4 5
# Default-Stop:	0 1 6
# Short-Description:	RBL client
### END INIT INFO

$HEAD


BIN=${PREFIX}/bin/ipbl
ETC_PATH=${PREFIX}/etc/ipbl
ACTION=\$1


get_config_files() {

	CONFIG_FILES=\`find \$ETC_PATH -maxdepth 1 -name *.conf\`
	# get files with theirs path include
}


get_ipbl_running_processes() {
	ps aux | grep \$BIN | egrep -v grep >/dev/null
	if [ $? ]; then
		PIDS=\`ps aux | grep \$BIN | egrep -v grep | awk '{ print \$2 }'\`
	fi
}


run_stop()
{
	get_ipbl_running_processes
	if [ \"x\$PIDS\" != \"x\" ]; then
		kill \$PIDS
	fi
		
}

run_start()
{
	get_config_files
	if [ \"x\$CONFIG_FILES\" != \"x\" ]; then
		for FILE in \$CONFIG_FILES
		do
			\$BIN -f \$FILE
		done
	fi
}


export PATH=\"\${PATH:+\$PATH:}/usr/sbin:/sbin\"

case \"\$ACTION\" in
  start)
	run_start
	;;
  stop)
	run_stop
	;;

  restart)
	run_stop
	run_start
	;;

  *)
	echo \"Usage: /etc/init.d/ipbl {start|stop|restart}\" 
	exit 1
esac

exit 0
" > $PACKAGE_DIR/etc/init.d/ipbl

chmod 755 $PACKAGE_DIR/etc/init.d/ipbl

mkdir -p ${PACKAGE_DIR}/$BIN_DIR
mkdir -p ${PACKAGE_DIR}/$ETC_DIR

cp ../bin/ipbl ${PACKAGE_DIR}/$BIN_DIR
cp -r example_config ${PACKAGE_DIR}/$ETC_DIR

# Create the Debian package
cd ..
dpkg-deb --build $PACKAGE_DIR_NAME
mv ${PACKAGE_DIR_NAME}.deb ipbl_${VERSION}.deb
rm -rf $PACKAGE_DIR_NAME

