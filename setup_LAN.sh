#!/bin/bash

# ===== Variables to modify if needed =====
WIFI_IFACE="wlp3s0"       # Wi-Fi interface name
ETH_IFACE="enp1s31f6"     # Ethernet interface for NAT
SSID="Chat-server"        # Wi-Fi SSID
WIFI_PASS="tpreseau"      # Wi-Fi password
SUBNET="192.168.0"        # Subnet for LAN
WIFI_IP="${SUBNET}.1"     # Static IP for Wi-Fi interface
DHCP_RANGE_START="${SUBNET}.10"   # DHCP range start
DHCP_RANGE_END="${SUBNET}.100"    # DHCP range end
DISABLE_OFFLOAD=1         # 1 = disable offloading (optional, improves compatibility)

# ===== Install required packages =====
sudo apt update
sudo apt install -y hostapd dnsmasq iptables

# ===== Unmask and enable hostapd (prevents "masked" issue) =====
sudo systemctl unmask hostapd
sudo systemctl enable hostapd

# ===== Function to reset the Wi-Fi interface =====
reset_wifi_iface() {
    # Stop hostapd before resetting interface
    sudo systemctl stop hostapd
    # Bring interface down and flush IPs
    sudo ip link set $WIFI_IFACE down
    sudo ip addr flush dev $WIFI_IFACE
    # Bring interface up and assign static IP
    sudo ip link set $WIFI_IFACE up
    sudo ip addr add $WIFI_IP/24 dev $WIFI_IFACE
    sleep 1
    # Restart hostapd service
    sudo systemctl start hostapd
}

# ===== Stop services before configuration =====
sudo systemctl stop hostapd
sudo systemctl stop dnsmasq

# ===== Configure static IP for Wi-Fi interface =====
sudo ip link set $WIFI_IFACE down
sudo ip addr flush dev $WIFI_IFACE
sudo ip link set $WIFI_IFACE up
sudo ip addr add $WIFI_IP/24 dev $WIFI_IFACE

# ===== Optional: disable offloading for Wi-Fi interface =====
if [ "$DISABLE_OFFLOAD" = "1" ]; then
    sudo ethtool -K $WIFI_IFACE gro off tso off gso off
fi

# ===== Configure dnsmasq (DHCP server) =====
sudo bash -c "cat > /etc/dnsmasq.conf" <<EOF
interface=$WIFI_IFACE
dhcp-range=$DHCP_RANGE_START,$DHCP_RANGE_END,255.255.255.0,24h
domain-needed
bogus-priv
EOF

# ===== Configure hostapd (Wi-Fi AP 2.4GHz) =====
sudo bash -c "cat > /etc/hostapd/hostapd.conf" <<EOF
interface=$WIFI_IFACE
ssid=$SSID
hw_mode=g
channel=6
beacon_int=100
wmm_enabled=1
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=$WIFI_PASS
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
EOF

# Point hostapd to its configuration file
sudo sed -i "s|#DAEMON_CONF=\"\"|DAEMON_CONF=\"/etc/hostapd/hostapd.conf\"|" /etc/default/hostapd

# ===== Enable IP forwarding and optimize network settings =====
sudo sysctl -w net.ipv4.ip_forward=1
sudo sysctl -w net.core.netdev_max_backlog=5000
sudo sysctl -w net.ipv4.tcp_congestion_control=bbr
sudo sed -i 's/#net.ipv4.ip_forward=1/net.ipv4.ip_forward=1/' /etc/sysctl.conf || true

# ===== Configure NAT using iptables =====
sudo iptables -t nat -A POSTROUTING -o $ETH_IFACE -j MASQUERADE
sudo iptables -A FORWARD -i $WIFI_IFACE -o $ETH_IFACE -j ACCEPT
sudo iptables -A FORWARD -i $ETH_IFACE -o $WIFI_IFACE -m state --state RELATED,ESTABLISHED -j ACCEPT

# ===== Save iptables rules for persistence =====
sudo sh -c "iptables-save > /etc/iptables.rules"
sudo bash -c 'cat > /etc/network/if-up.d/iptables' <<EOF
#!/bin/sh
iptables-restore < /etc/iptables.rules
EOF
sudo chmod +x /etc/network/if-up.d/iptables

# ===== Start dnsmasq and hostapd services =====
sudo systemctl restart dnsmasq
reset_wifi_iface

# ===== Auto-restart hostapd on crash =====
sudo mkdir -p /etc/systemd/system/hostapd.service.d
sudo bash -c 'cat > /etc/systemd/system/hostapd.service.d/override.conf' <<EOF
[Service]
Restart=always
RestartSec=2
EOF
sudo systemctl daemon-reload

# ===== Display summary information =====
echo "✅ Access Point ready!"
echo "SSID: $SSID"
echo "Password: $WIFI_PASS"
echo "Wi-Fi Interface: $WIFI_IFACE"
echo "AP IP: $WIFI_IP"
echo "DHCP: ${DHCP_RANGE_START} → ${DHCP_RANGE_END}"
echo "Hostapd auto-restart enabled. For manual reset: reset_wifi_iface"
