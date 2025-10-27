# DWPC Doorway Sensor - Enterprise Network Debug Guide

## Problem Summary

**Symptoms:**
- Device takes ~8 hours to come fully online in enterprise client environment
- Works immediately in local network after reboot
- Ping shows IP unavailable except for 1-2 seconds intermittently
- Client uses: Ethernet mode, Static IP, VLAN with subnet mask and gateway

**Environment:**
- Local Network: Device comes online immediately ✓
- Enterprise Network: Takes ~8 hours, intermittent connectivity ✗

---

## Root Cause Analysis

### CRITICAL BUG #1: WiFi Auto-Restart Loop

**Location:** `src/main.c:753-776` in `periodicResetCount()` function

**The Bug:**
```c
bool is_wifi_enabled = mgos_sys_config_get_wifi_sta_enable();
if (is_wifi_enabled)
{
    int wifi_connection_status = mgos_wifi_get_status();
    if (wifi_connection_status != 3)  // Not connected
    {
        mgos_wifi_connect();
        wifi_connection_status_counter++;
    }

    if (wifi_connection_status_counter > 2)  // After 3 failures
    {
        mgos_system_restart();  // RESTARTS ENTIRE DEVICE!
    }
}
```

**Why This Causes 8-Hour Delay:**

1. Even when using Ethernet, if WiFi STA is enabled in config (but not connected), the device checks WiFi status
2. WiFi fails 3 consecutive times (every 45 minutes by default)
3. After 3 failures (~2.25 hours), the ENTIRE device restarts
4. This creates an **infinite boot loop**: Boot → Run 2.25hrs → WiFi Check Fails → Restart → Repeat
5. Over multiple cycles (3-4 restarts), eventually network stabilizes ≈ **8 hours total**

**The 1-2 Second Ping Availability:**
- Device boots, Ethernet comes up briefly
- Responds to ping for 1-2 seconds
- WiFi check triggers restart → Device goes offline
- Cycle repeats, creating intermittent connectivity

---

### Issue #2: No Ethernet Link Status Monitoring

**Problem:** The firmware only monitors WiFi connection status, not Ethernet link status.

**Missing Functionality:**
- No check if Ethernet cable is plugged in
- No check if Ethernet link is UP
- No validation that Ethernet has acquired IP address
- No gateway reachability test

**Result:** Device doesn't know if Ethernet is working, relies only on WiFi status checks.

---

### Issue #3: Enterprise Network Compatibility

**Spanning Tree Protocol (STP) Delays:**
- Enterprise switches run STP for loop prevention
- STP can take **30-50 seconds** to transition a port to forwarding state
- Device might boot and try to communicate before switch port is ready
- Results in failed initialization, triggers restart loop

**Port Security & MAC Learning:**
- Enterprise switches validate MAC addresses
- May have port security enabled (max MAC addresses per port)
- Static MAC assignment or whitelisting required
- Device MAC changes during restarts can cause delays

**VLAN Configuration:**
- Client uses VLAN with specific subnet/gateway
- Inter-VLAN routing may have security policies
- ACLs might be blocking initial traffic
- DHCP snooping enabled even with static IP

**ARP Issues:**
- Device doesn't send Gratuitous ARP on startup
- Switch ARP cache might be stale
- Gateway may not know device MAC address
- Results in "IP exists but unreachable" symptom

---

### Issue #4: No Network Diagnostic Logging

**Missing Diagnostics:**
- No Ethernet link status logs
- No gateway reachability tests (ping)
- No ARP table logging
- No detailed startup sequence logs
- Hard to debug what's failing

---

## Immediate Debug Steps

### Step 1: Check Current WiFi Configuration

Connect to device and check:

```bash
mos config-get wifi.sta.enable
mos config-get wifi.ap.enable
mos config-get eth.enable
mos config-get eth.ip
mos config-get eth.netmask
mos config-get eth.gw
```

**Expected for Ethernet-only:**
- `wifi.sta.enable`: false (MUST be false to avoid restart loop!)
- `wifi.ap.enable`: true or false (doesn't matter)
- `eth.enable`: true
- `eth.ip`: Your static IP
- `eth.netmask`: Your subnet mask
- `eth.gw`: Your gateway IP

### Step 2: Disable WiFi STA Mode

If WiFi STA is enabled, disable it immediately:

```bash
mos config-set wifi.sta.enable=false
mos config-set eth.enable=true
mos config-set eth.ip="192.168.1.100"  # Your static IP
mos config-set eth.netmask="255.255.255.0"  # Your subnet
mos config-set eth.gw="192.168.1.1"  # Your gateway
```

Then reboot:

```bash
mos call Sys.Reboot
```

### Step 3: Monitor Logs During Boot

Watch the device logs during startup:

```bash
mos console
```

Look for:
- "is_wifi_enabled ########################" message (should show 0)
- Ethernet initialization messages
- MQTT connection status
- Any restart messages

### Step 4: Test in Client Environment

After disabling WiFi STA:

1. Deploy to client site
2. Power on device
3. Monitor with continuous ping:
   ```bash
   ping -i 1 <device_ip>  # Linux/Mac
   ping -t <device_ip>     # Windows
   ```
4. Watch for consistent responses (should come online in 30-60 seconds, not 8 hours)

### Step 5: Check Enterprise Network Settings

Work with client IT to verify:

**Switch Port Configuration:**
- Is PortFast enabled? (Skips STP delay)
- Is there port security? (Provide device MAC)
- Is there a VLAN assigned? (Correct VLAN?)
- Is there an ACL blocking traffic?

**Network Policies:**
- Is static IP allowed on this VLAN?
- Is there DHCP snooping? (Can interfere with static IP)
- Is there IP source guard?
- Are there any MAC filtering policies?

**Gateway/Routing:**
- Can you ping the gateway from device?
- Is there inter-VLAN routing configured?
- Are there firewall rules blocking device traffic?

---

## Permanent Fixes Required

### Fix #1: Resolve WiFi Auto-Restart Bug (CRITICAL)

**Modify:** `src/main.c:753-776`

**Change from:**
```c
bool is_wifi_enabled = mgos_sys_config_get_wifi_sta_enable();
if (is_wifi_enabled)
{
    // Check WiFi and restart device if fails
}
```

**Change to:**
```c
// Only check WiFi connection if WiFi is PRIMARY interface (Ethernet disabled)
bool is_wifi_enabled = mgos_sys_config_get_wifi_sta_enable();
bool is_eth_enabled = mgos_sys_config_get_eth_enable();

// Only restart on WiFi failure if Ethernet is NOT being used
if (is_wifi_enabled && !is_eth_enabled)
{
    // Check WiFi and restart device if fails
}
else if (is_wifi_enabled && is_eth_enabled)
{
    // WiFi is secondary, don't restart device if WiFi fails
    // Just log the failure
    printf("WiFi is secondary interface, not enforcing connection\n");
}
```

### Fix #2: Add Ethernet Link Status Monitoring

**Add to:** `src/main.c:periodicResetCount()`

```c
// Check Ethernet status if enabled
bool is_eth_enabled = mgos_sys_config_get_eth_enable();
if (is_eth_enabled)
{
    // TODO: Add Mongoose OS Ethernet status check
    // Monitor Ethernet link status, IP acquisition, gateway reachability
    printf("Ethernet is enabled and primary interface\n");
}
```

### Fix #3: Add STP-Aware Startup Delay

**Add to:** `src/main.c:mgos_app_init()`

Add a configurable delay after Ethernet initialization to wait for STP:

```c
// Wait for enterprise switch STP to enable port (30-50 seconds typical)
int eth_startup_delay = mgos_sys_config_get_eth_startup_delay();
if (eth_startup_delay > 0)
{
    printf("Waiting %d seconds for network port to come online (STP)...\n", eth_startup_delay);
    mgos_msleep(eth_startup_delay * 1000);
}
```

**Add to:** `mos.yml` config schema:

```yaml
- ["eth.startup_delay", "i", 0, {title: "Startup delay in seconds for STP (0=disabled, 30-60 for enterprise)"}]
```

### Fix #4: Add Gratuitous ARP Broadcast

Send Gratuitous ARP on startup to announce device presence:

```c
// After Ethernet is initialized, send Gratuitous ARP
// This tells the switch/gateway about device MAC address
// Helps with stale ARP cache issues
```

### Fix #5: Add Network Diagnostic Logging

**Add periodic network health checks:**

```c
void network_diagnostics()
{
    // Log Ethernet link status
    // Log IP configuration
    // Test gateway reachability (ping)
    // Log ARP table
    // Log MQTT connection status
    // Log uptime without restarts
}
```

### Fix #6: Add Configuration Validation

Prevent invalid configurations:

```c
// Validate: At least one network interface must be enabled
bool wifi_sta = mgos_sys_config_get_wifi_sta_enable();
bool wifi_ap = mgos_sys_config_get_wifi_ap_enable();
bool eth = mgos_sys_config_get_eth_enable();

if (!wifi_sta && !wifi_ap && !eth)
{
    printf("ERROR: No network interface enabled! Enabling WiFi AP fallback.\n");
    mgos_config_set_wifi_ap_enable(true);
}
```

---

## Configuration Best Practices

### For Ethernet-Only Deployment:

```yaml
# WiFi Configuration
wifi.sta.enable: false          # CRITICAL: Must be false!
wifi.ap.enable: false           # Disable AP after initial setup
wifi.ap.keep_enabled: false

# Ethernet Configuration
eth.enable: true
eth.ip: "192.168.1.100"         # Your static IP
eth.netmask: "255.255.255.0"    # Your subnet mask
eth.gw: "192.168.1.1"           # Your gateway
eth.startup_delay: 45           # Wait 45s for STP (enterprise networks)

# MQTT Configuration
mqtt.enable: true
mqtt.server: "your.mqtt.broker"

# Periodic Reset (health check interval)
dwpc.periodic_reset: 45         # 45 minutes default
```

### For WiFi + Ethernet Dual Mode:

```yaml
wifi.sta.enable: true
eth.enable: true
# Device will prefer Ethernet if both available
# Won't restart if WiFi fails while Ethernet works
```

---

## Testing Checklist

### Local Network Test:
- [ ] Device boots within 30 seconds
- [ ] Ping responds immediately
- [ ] MQTT connects within 1 minute
- [ ] No unexpected restarts

### Enterprise Network Test:
- [ ] Device boots within 60 seconds (accounting for STP)
- [ ] Ping responds consistently after initial delay
- [ ] MQTT connects successfully
- [ ] No restart loops
- [ ] Uptime continues to increase (check with mos call Sys.GetInfo)

### Configuration Test:
- [ ] WiFi STA disabled (wifi.sta.enable = false)
- [ ] Ethernet enabled with correct static IP
- [ ] Periodic reset NOT causing device restarts
- [ ] Logs show "WiFi is disabled, no need to check wifi connection status"

---

## Quick Reference Commands

```bash
# Check device configuration
mos config-get

# Disable WiFi STA (CRITICAL FIX)
mos config-set wifi.sta.enable=false

# Configure Ethernet static IP
mos config-set eth.enable=true
mos config-set eth.ip="192.168.1.100"
mos config-set eth.netmask="255.255.255.0"
mos config-set eth.gw="192.168.1.1"

# Set STP delay for enterprise networks
mos config-set eth.startup_delay=45

# Check device uptime (should increase, not reset)
mos call Sys.GetInfo

# Reboot device
mos call Sys.Reboot

# Monitor logs
mos console

# Check MQTT status
mos config-get mqtt.status
```

---

## Expected Behavior After Fix

1. **Boot Time:**
   - Local network: 5-10 seconds
   - Enterprise network: 30-60 seconds (STP delay)

2. **Ping Response:**
   - Consistent responses after boot
   - No intermittent drops
   - No 8-hour delay

3. **Uptime:**
   - Continuously increases
   - No automatic restarts
   - Stable operation

4. **Logs:**
   - "WiFi is disabled, no need to check wifi connection status"
   - "Ethernet is enabled and primary interface"
   - No "mgos_system_restart" messages

---

## Contact Information

If issues persist after applying fixes:

1. Collect full device logs during boot
2. Capture network packet trace (Wireshark)
3. Document switch configuration (port settings, VLAN, ACLs)
4. Check client firewall/security policies

---

## Version History

- v1.0 (2025-10-27): Initial debug guide
- Root cause: WiFi auto-restart bug
- Solution: Disable WiFi STA or fix restart logic
