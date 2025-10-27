# DWPC Enterprise Deployment Guide

## Quick Fix for 8-Hour Delay Issue

This firmware update resolves the critical boot loop issue that caused 8-hour delays in enterprise networks.

---

## What Changed

### Critical Bug Fix
**WiFi Auto-Restart Loop** - Fixed logic that was restarting the device every 2.25 hours when WiFi was enabled but Ethernet was being used. This was causing infinite boot loops in enterprise environments.

### New Features
1. **Enterprise STP Support** - Configurable startup delay for Spanning Tree Protocol compatibility
2. **Enhanced Network Diagnostics** - Detailed logging of network status and configuration
3. **Ethernet Status Monitoring** - Real-time monitoring of Ethernet connection
4. **Improved MQTT Logging** - Better visibility into connection status

---

## Configuration for Enterprise Ethernet Deployment

### Recommended Configuration

```bash
# Disable WiFi STA (CRITICAL!)
mos config-set wifi.sta.enable=false

# Configure Ethernet with static IP
mos config-set eth.enable=true
mos config-set eth.ip="YOUR_STATIC_IP"           # e.g., "192.168.100.50"
mos config-set eth.netmask="YOUR_SUBNET_MASK"    # e.g., "255.255.255.0"
mos config-set eth.gw="YOUR_GATEWAY_IP"          # e.g., "192.168.100.1"

# Set STP delay for enterprise switches (30-60 seconds recommended)
mos config-set eth.startup_delay=45

# Configure MQTT server
mos config-set mqtt.server="YOUR_MQTT_BROKER"
mos config-set mqtt.pub="/dwpc/YOUR_ID"

# Reboot device
mos call Sys.Reboot
```

### Configuration File Template

Save as `device_config.txt`:

```
wifi.sta.enable=false
wifi.ap.enable=false
eth.enable=true
eth.ip=192.168.100.50
eth.netmask=255.255.255.0
eth.gw=192.168.100.1
eth.startup_delay=45
mqtt.enable=true
mqtt.server=mqtt.yourdomain.com
mqtt.pub=/dwpc/sensor01
```

Apply configuration:
```bash
mos config-set < device_config.txt
mos call Sys.Reboot
```

---

## Expected Behavior After Fix

### Boot Time
- **Local Network**: 5-10 seconds
- **Enterprise Network**: 30-60 seconds (depending on STP delay setting)

### Network Status
- **Ping Response**: Consistent and immediate after boot
- **MQTT Connection**: Within 30 seconds of boot
- **No Restarts**: Device uptime should continuously increase

### Console Logs
You should see messages like:
```
========================================
DWPC Network Configuration
========================================
Device MAC: AABBCCDDEEFF
WiFi STA: DISABLED
Ethernet: ENABLED
  Static IP: 192.168.100.50
  Netmask: 255.255.255.0
  Gateway: 192.168.100.1
  STP Delay: 45 seconds
MQTT Server: mqtt.yourdomain.com
========================================

Enterprise Network Mode: Waiting 45 seconds for switch port to come online (STP)
Waiting for network... 45 seconds remaining
Waiting for network... 40 seconds remaining
...
Network wait complete. Proceeding with initialization.
```

Later during operation:
```
=== Ethernet Network Status ===
Ethernet enabled: true
Static IP: 192.168.100.50
Netmask: 255.255.255.0
Gateway: 192.168.100.1
Device uptime: 3600 seconds
================================
MQTT Status: Connected to mqtt.yourdomain.com
```

---

## Verification Steps

### 1. Check Configuration
```bash
mos console
# Look for "DWPC Network Configuration" banner at boot
# Verify WiFi STA shows "DISABLED"
# Verify Ethernet shows correct IP/Gateway
```

### 2. Monitor Boot Process
```bash
# In another terminal, continuously ping the device
ping -i 1 192.168.100.50

# Watch console for startup messages
mos console
```

**Expected Result**:
- Console shows STP delay countdown
- Ping starts responding after STP delay completes
- No restart messages
- Uptime increases continuously

### 3. Check Uptime After 1 Hour
```bash
mos call Sys.GetInfo
```

**Expected Result**: Uptime should be ~3600 seconds (1 hour), not reset to 0

### 4. Verify MQTT Connection
```bash
# Subscribe to your MQTT topic
mosquitto_sub -h YOUR_MQTT_BROKER -t "/dwpc/YOUR_ID"
```

**Expected Result**: Should see people counting data within 1 minute of boot

---

## Troubleshooting

### Issue: Device still restarting every 2 hours
**Cause**: WiFi STA is still enabled
**Solution**:
```bash
mos config-set wifi.sta.enable=false
mos call Sys.Reboot
```

### Issue: Device not responding to ping after boot
**Cause**: STP delay might be too short
**Solution**: Increase STP delay
```bash
mos config-set eth.startup_delay=60
mos call Sys.Reboot
```

### Issue: MQTT not connecting
**Cause**: Network routing or firewall
**Solution**:
1. Verify device can reach gateway (check logs for "Ethernet Network Status")
2. Check client firewall rules
3. Verify MQTT broker is accessible from device VLAN

### Issue: Intermittent connectivity (1-2 seconds)
**Cause**: MAC address conflict or ARP issues
**Solution**:
1. Verify no IP address conflicts on network
2. Clear ARP cache on gateway
3. Check switch MAC address table
4. Verify no duplicate MAC addresses

---

## Enterprise Network Checklist

Work with client IT to verify:

### Switch Port Configuration
- [ ] PortFast enabled (bypasses STP delay) OR STP delay configured in firmware
- [ ] No port security restrictions
- [ ] Correct VLAN assigned
- [ ] No MAC filtering

### Network Configuration
- [ ] Static IP is within allowed range
- [ ] No DHCP snooping conflicts
- [ ] No IP source guard restrictions
- [ ] Gateway is reachable from VLAN

### Firewall/Security
- [ ] Device can reach MQTT broker (port 1883 or 8883)
- [ ] No ACLs blocking device traffic
- [ ] NTP server accessible (time.google.com or internal NTP)

---

## Firmware Build and Flash

### Building Firmware
```bash
cd /home/user/dwpc
mos build --platform esp32
```

### Flashing Device
```bash
mos flash
```

### Flashing Over Network (OTA)
```bash
# From device with old firmware
mos flash-boot-fw
```

---

## Default vs Enterprise Configuration

| Setting | Default | Enterprise Ethernet |
|---------|---------|---------------------|
| wifi.sta.enable | false | **false** (CRITICAL) |
| wifi.ap.enable | true | false (after setup) |
| eth.enable | true | **true** |
| eth.ip | "" | **Static IP** |
| eth.startup_delay | 0 | **30-60 seconds** |
| mqtt.server | test.mosquitto.org | **Client MQTT broker** |
| dwpc.periodic_reset | 45 min | 45 min (no restart) |

---

## Support

For detailed debugging information, see:
- `NETWORK_DEBUG_GUIDE.md` - Comprehensive debugging guide
- `README.md` - Project overview

Check device logs:
```bash
mos console
```

Get device info:
```bash
mos call Sys.GetInfo
```

Get full configuration:
```bash
mos config-get
```

---

## Version Information

- **Firmware**: db-occusenz-pc-l8-v3.10-23062025-dc
- **Fix Date**: 2025-10-27
- **Issue**: 8-hour boot delay in enterprise networks
- **Root Cause**: WiFi auto-restart loop
- **Status**: RESOLVED
