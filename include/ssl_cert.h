#ifndef SSL_CERT_H
#define SSL_CERT_H

#include <Arduino.h>
#include <Preferences.h>
#include "mbedtls/pk.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/oid.h"
#include "mbedtls/error.h"

class SSLCertificate {
private:
    Preferences preferences;
    
    // Certificate data
    String certificatePEM;
    String privateKeyPEM;
    unsigned long expiryTime;
    bool certificateValid;
    
    // Internal methods
    bool generateNewCertificate();
    bool loadStoredCertificate();
    bool saveCertificateToFlash();
    bool checkCertificateExpiry() const;  // Made const
    
    // Crypto helpers
    int generateRSAKeyPair(mbedtls_pk_context &key);
    int createX509Certificate(mbedtls_pk_context &key, String &certPEM, String &keyPEM);
    
    // Constants
    static const int RSA_KEY_SIZE = 2048;
    static const int CERT_VALIDITY_DAYS = 365;  // 1 year validity
    static const unsigned long RENEWAL_THRESHOLD = 30 * 24 * 60 * 60; // Renew 30 days before expiry
    
public:
    SSLCertificate();
    ~SSLCertificate();
    
    // Main interface
    bool initialize();
    bool ensureValidCertificate();
    void regenerateIfNeeded();
    
    // Certificate access
    String getCertificatePEM() const;
    String getPrivateKeyPEM() const;
    const char* getCertificateData() const;
    const char* getPrivateKeyData() const;
    
    // Status checking
    bool isValid() const;
    bool isExpiringSoon() const;
    unsigned long getExpiryTime() const;
    unsigned long getDaysUntilExpiry() const;
    
    // Utility
    void printCertificateInfo() const;
    bool clearStoredCertificate();
    
    // Error handling
    String getLastError() const;
    
private:
    String lastError;
};

#endif // SSL_CERT_H