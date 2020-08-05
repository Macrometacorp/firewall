# firewall
Firewall based on PacketBL

How to build and install:

You should have received an email from macrometa. 
Please copy the app key and the api url at the end of example_config/ipbl.conf.

Then compile the package:

cd src
./configure
make
make deb
sudo dpkg -i ../ipbl_1.0.deb

use this command to setup the hook:

iptables -A INPUT -t filter -p tcp --dport 25 -j NFQUEUE --queue-num 0

