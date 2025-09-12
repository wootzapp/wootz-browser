# Wootz Hardware-Backed Certificate System

## Overview

The Wootz Hardware-Backed Certificate System provides device-level security using Android's hardware security modules (Strongbox/TEE) to generate and store cryptographic keys that cannot be exported or accessed by software. This system establishes a unique device identity for secure browser operations.

## Architecture

### Security Boundary Design

```
┌─────────────────────────────────────────────────────────┐
│                 Chrome Browser Layer                    │
│  ┌─────────────────────────────────────────────────────┐ │
│  │        ProcessInitializationHandler.java           │ │
│  │        (Calls initialization once per browser)     │ │
│  └─────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────┐
│                 net/android (Trusted Layer)            │
│  ┌─────────────────────────────────────────────────────┐ │
│  │           WootzHardwareKeyStore.java                │ │
│  │     - Public: initializeHardwareKeystore()         │ │
│  │     - Private: @CalledByNative JNI methods         │ │
│  │     - Security: Non-exportable key operations      │ │
│  └─────────────────────────────────────────────────────┘ │
│  ┌─────────────────────────────────────────────────────┐ │
│  │              wootz_keystore.h/.cc                  │ │
│  │            (C++ JNI Bridge - Future)               │ │
│  └─────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────┐
│              Android KeyStore (Hardware)               │
│  ┌─────────────────┐    ┌─────────────────────────────┐ │
│  │   Strongbox     │    │        TEE (TrustZone)      │ │
│  │  (Dedicated HSM)│    │   (Trusted Execution Env)  │ │
│  │                 │    │                             │ │
│  │ - Highest Sec   │    │    - High Security          │ │
│  │ - Tamper Resist │    │    - Software Isolated     │ │
│  │ - Key Isolation │    │    - Hardware Backed       │ │
│  └─────────────────┘    └─────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

### Why net/android vs chrome/browser?

The implementation is placed in `net/android` rather than `chrome/browser` for critical security reasons:

1. **Trust Boundary**: `net/android` is part of Chromium's trusted networking layer, closer to system security services
2. **Attack Surface**: Reduces attack surface by keeping cryptographic operations away from browser UI/content layer
3. **Process Isolation**: Better isolation from renderer processes and web content
4. **Security Auditing**: Easier to audit and secure a smaller, focused codebase
5. **Platform Integration**: Direct access to Android platform security APIs

## Implementation Details

### Files Structure

```
net/android/
├── java/src/org/chromium/net/
│   └── WootzHardwareKeyStore.java          # Main implementation
├── wootz_keystore.h                        # C++ header (future JNI bridge)
├── wootz_keystore.cc                       # C++ implementation (future)
└── BUILD.gn                                # Build configuration

chrome/android/java/src/org/chromium/chrome/browser/init/
└── ProcessInitializationHandler.java       # Initialization trigger
```

### Key Components

#### 1. WootzHardwareKeyStore.java

**Purpose**: Provides hardware-backed cryptographic operations using Android KeyStore

**Key Features**:
- **Hardware Key Generation**: Creates EC P-256 keys in Strongbox/TEE
- **Attestation Support**: Generates hardware attestation certificates
- **Security Level Detection**: Identifies Strongbox vs TEE backing
- **Non-Exportable Keys**: Private keys cannot be extracted from hardware
- **JNI-Ready**: @CalledByNative methods for future C++ integration

**Public API**:
```java
// Application-level initialization (called once per browser startup)
public static boolean initializeHardwareKeystore()

// Security level queries
public static boolean isKeyStrongboxBacked()
public static boolean isKeyHardwareBacked()

// Certificate access
public static String getDevicePublicKeyPem()
public static String getAttestationChainPem()
```

**Private JNI Methods**:
```java
// Core cryptographic operations (C++ accessible only)
@CalledByNative private static boolean generateHardwareBackedKeyWithAttestation(byte[])
@CalledByNative private static byte[] signWithHardwareKey(byte[])
```

#### 2. ProcessInitializationHandler.java

**Purpose**: Browser-level initialization that runs once per process lifetime

**Integration Point**:
```java
protected void handlePostNativeInitialization() {
    // Initialize hardware-backed certificate system first
    WootzHardwareKeyStore.initializeHardwareKeystore();
    
    // ... other browser initialization
}
```

**Why This Location**:
- Runs once per browser process (not per activity/tab)
- Executes after native library loading
- Independent of user profiles or FirstRun experience
- Proper timing for system-level security setup

## Security Properties

### Hardware Security Levels

1. **Strongbox (Highest Security)**
   - Dedicated Hardware Security Module (HSM)
   - Tamper-resistant hardware
   - Physical key isolation
   - Side-channel attack resistance

2. **TEE - Trusted Execution Environment**
   - TrustZone-based secure world
   - Software isolation from Android OS
   - Hardware memory protection
   - Secure boot chain verification

3. **Software Fallback (Not Used)**
   - Keys stored in Android KeyStore software implementation
   - Not suitable for high-security applications

### Key Properties

- **Algorithm**: NIST P-256 Elliptic Curve
- **Key Usage**: Digital Signature, Key Agreement
- **Attestation**: Hardware attestation certificate chain
- **Non-Exportable**: Private key never leaves secure hardware
- **Unique Per Device**: Each device generates its own keypair

### Attack Resistance

- **Root/Malware**: Even with root access, private key cannot be extracted
- **Physical Access**: Strongbox provides tamper resistance
- **Side-Channel**: Hardware implementations resist timing/power analysis
- **Software Bugs**: Private key operations isolated from application bugs

## Initialization Flow

### Browser Startup Sequence

```
1. Chrome Application Launch
   ↓
2. ProcessInitializationHandler.initializePreNative()
   ↓
3. Native Library Loading
   ↓
4. ProcessInitializationHandler.handlePostNativeInitialization()
   ↓
5. WootzHardwareKeyStore.initializeHardwareKeystore()
   ↓
6. Check if hardware key already exists
   ↓
7a. Key Exists → Log security level, continue
7b. No Key → Generate new hardware-backed key with attestation
   ↓
8. Browser initialization complete with device identity established
```

### First Run vs Subsequent Runs

**First Run**:
- Generates new hardware-backed keypair
- Creates attestation certificate chain
- Stores key permanently in Android KeyStore
- Logs security level (Strongbox/TEE)

**Subsequent Runs**:
- Detects existing hardware key
- Skips key generation
- Validates key accessibility
- Continues with existing device identity

## Usage Examples

### Checking Security Level

```java
if (WootzHardwareKeyStore.isKeyStrongboxBacked()) {
    Log.i(TAG, "Maximum security: Strongbox HSM");
} else if (WootzHardwareKeyStore.isKeyHardwareBacked()) {
    Log.i(TAG, "High security: TEE/TrustZone");
} else {
    Log.w(TAG, "Software-only security");
}
```

### Getting Device Certificate

```java
String devicePublicKey = WootzHardwareKeyStore.getDevicePublicKeyPem();
String attestationChain = WootzHardwareKeyStore.getAttestationChainPem();

// Use for device authentication, secure communications, etc.
```

## Build Integration

### Android Build Configuration

**net/android/BUILD.gn**:
```gn
android_library("java") {
  sources = [
    "java/src/org/chromium/net/WootzHardwareKeyStore.java",
  ]
  deps = [
    "//base:base_java",
    "//base:jni_java",
  ]
}
```

**Generated JNI Headers**:
- `net/net_jni_headers/WootzHardwareKeyStore_jni.h` (auto-generated)

## Future Enhancements

### Planned Features

1. **C++ JNI Bridge**: Complete wootz_keystore.cc implementation for C++ access
2. **Certificate Chain Validation**: Verify attestation certificate authenticity
3. **Key Rotation**: Periodic key renewal with continuity
4. **Remote Attestation**: Server-side device verification
5. **Secure Channel**: TLS client certificate authentication

### API Extensions

```java
// Future public APIs
public static boolean rotateHardwareKey()
public static boolean verifyAttestationChain()
public static SSLContext createHardwareBackedSSLContext()
```

## Security Considerations

### Threat Model

**Protected Against**:
- Malware with root privileges
- Physical device access (Strongbox)
- Software vulnerabilities in browser
- Memory dumps/debugging
- Key extraction attempts

**Not Protected Against**:
- Compromised secure hardware (theoretical)
- Nation-state hardware implants
- Sophisticated side-channel attacks on TEE
- Physical destruction of device

### Best Practices

1. **Always check security level** before critical operations
2. **Log security events** for monitoring
3. **Handle initialization failures** gracefully
4. **Never cache private key material** in memory
5. **Use proper attestation validation** in production

## Debugging and Troubleshooting

### Common Issues

**Key Generation Failures**:
- Check device hardware security support
- Verify Android version compatibility (API 23+)
- Ensure sufficient secure storage space

**Strongbox Not Available**:
- Fallback to TEE is automatic
- Check device specifications
- Log actual security level used

### Logging

Enable hardware keystore logging:
```java
Log.setLevel(Log.VERBOSE);
// Look for "WootzHardwareKeyStore" tag in logcat
```

### Testing

**Device Compatibility Testing**:
```bash
# Check hardware security features
adb shell getprop ro.hardware.keystore
adb shell getprop ro.hardware.keystore_desede

# Monitor key operations
adb logcat | grep WootzHardwareKeyStore
```

## Compliance and Standards

### Standards Compliance

- **FIDO2/WebAuthn**: Hardware-backed authenticator support
- **Android Keystore**: Full compatibility with Android security APIs
- **NIST Guidelines**: P-256 curve, secure key generation
- **Common Criteria**: Hardware security module requirements

### Privacy Considerations

- Device identity is hardware-based but not user-linked
- No personally identifiable information in keys
- Attestation certificates contain only hardware information
- User consent not required for hardware key generation

## References

- [Android Hardware-backed Keystore](https://source.android.com/security/keystore)
- [Android Keystore System](https://developer.android.com/training/articles/keystore)
- [Hardware Security Module](https://source.android.com/security/keystore/implementer-ref)
- [Strongbox Implementation](https://source.android.com/security/keystore/strongbox)
