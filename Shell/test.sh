# 向test.log写入, 以EOF结尾

cat > ./test.log <<EOF
######################################################################
# /etc/network/interfaces -- configuration file for ifup(8), ifdown(8)
# See the interfaces(5) manpage for information on what options are
# available.
######################################################################

# We always want the loopback interface.
#
auto lo
iface lo inet loopback

# A. For DHCP on eth0
# auto eth0
# iface eth0 inet dhcp

# B. For static on eth0
 auto eth0
 iface eth0 inet static
     address 192.168.137.42
#     network 192.168.1.0
     netmask 255.255.255.0
#     broadcast 192.168.1.255
     gateway 192.168.137.1

auto eth1
 iface eth1 inet static
     address 192.168.0.42
#     network 192.168.0.0
     netmask 255.255.255.0
#     broadcast 192.168.0.255
     gateway 192.168.0.1

auto eth2
 iface eth2 inet static
     address 192.168.2.42
#     network 192.168.2.0
     netmask 255.255.255.0
#     broadcast 192.168.2.255
     gateway 192.168.2.1

#dns-nameservers 8.8.8.8

EOF
