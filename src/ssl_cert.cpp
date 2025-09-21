#include "ssl_cert.h"

SSLCertificate::SSLCertificate() : certificateValid(false), expiryTime(0) {
}

SSLCertificate::~SSLCertificate() {
    preferences.end();
}

bool SSLCertificate::initialize() {
    Serial.println("Initializing SSL Certificate module...");
    
    // Initialize preferences
    if (!preferences.begin("ssl_cert", false)) {
        lastError = "Failed to initialize preferences";
        Serial.println("Error: " + lastError);
        return false;
    }
    
    // Try to load existing certificate
    if (loadStoredCertificate()) {
        Serial.println("Loaded existing certificate from flash");
        
        // Check if it's still valid
        if (checkCertificateExpiry()) {
            Serial.println("Certificate is valid");
            certificateValid = true;
            return true;
        } else {
            Serial.println("Stored certificate has expired, will generate new one");
        }
    } else {
        Serial.println("No valid certificate found in flash");
    }
    
    // Generate new certificate if needed
    return ensureValidCertificate();
}

bool SSLCertificate::ensureValidCertificate() {
    if (certificateValid && !isExpiringSoon()) {
        return true;
    }
    
    Serial.println("Generating new SSL certificate...");
    
    if (!generateNewCertificate()) {
        Serial.println("Error: Failed to generate certificate");
        return false;
    }
    
    if (!saveCertificateToFlash()) {
        Serial.println("Warning: Failed to save certificate to flash");
        // Continue anyway - certificate is still valid in memory
    }
    
    certificateValid = true;
    Serial.println("SSL certificate ready!");
    return true;
}

void SSLCertificate::regenerateIfNeeded() {
    if (!certificateValid || isExpiringSoon()) {
        Serial.println("Certificate needs regeneration...");
        ensureValidCertificate();
    }
}

bool SSLCertificate::generateNewCertificate() {
    mbedtls_pk_context key;
    mbedtls_pk_init(&key);
    
    Serial.println("Generating RSA key pair...");
    
    // Generate RSA key pair
    if (generateRSAKeyPair(key) != 0) {
        mbedtls_pk_free(&key);
        return false;
    }
    
    Serial.println("Creating X.509 certificate...");
    
    // Create certificate
    if (createX509Certificate(key, certificatePEM, privateKeyPEM) != 0) {
        mbedtls_pk_free(&key);
        return false;
    }
    
    // Set expiry time (current time + validity period)
    expiryTime = millis() + (CERT_VALIDITY_DAYS * 24UL * 60UL * 60UL * 1000UL);
    
    mbedtls_pk_free(&key);
    
    Serial.println("Certificate generated successfully!");
    printCertificateInfo();
    
    return true;
}

int SSLCertificate::generateRSAKeyPair(mbedtls_pk_context &key) {
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    
    const char *pers = "ssl_cert_gen";
    int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                   (const unsigned char *) pers, strlen(pers));
    
    if (ret != 0) {
        lastError = "Failed to seed random number generator: " + String(ret);
        goto cleanup;
    }
    
    ret = mbedtls_pk_setup(&key, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));
    if (ret != 0) {
        lastError = "Failed to setup PK context: " + String(ret);
        goto cleanup;
    }
    
    Serial.printf("Generating %d-bit RSA key...\n", RSA_KEY_SIZE);
    ret = mbedtls_rsa_gen_key(mbedtls_pk_rsa(key), mbedtls_ctr_drbg_random, &ctr_drbg,
                             RSA_KEY_SIZE, 65537);
    
    if (ret != 0) {
        lastError = "Failed to generate RSA key: " + String(ret);
        goto cleanup;
    }
    
cleanup:
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    
    return ret;
}

int SSLCertificate::createX509Certificate(mbedtls_pk_context &key, String &certPEM, String &keyPEM) {
    mbedtls_x509write_cert crt;
    mbedtls_mpi serial;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    
    unsigned char cert_buf[4096];
    unsigned char key_buf[4096];
    
    mbedtls_x509write_crt_init(&crt);
    mbedtls_mpi_init(&serial);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    
    const char *pers = "crt_gen";
    int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                   (const unsigned char *) pers, strlen(pers));
    
    if (ret != 0) {
        lastError = "Failed to seed RNG for certificate: " + String(ret);
        goto cleanup;
    }
    
    // Set certificate version (X.509 v3)
    mbedtls_x509write_crt_set_version(&crt, MBEDTLS_X509_CRT_VERSION_3);
    
    // Set serial number
    ret = mbedtls_mpi_read_string(&serial, 10, "1");
    if (ret != 0) goto cleanup;
    
    ret = mbedtls_x509write_crt_set_serial(&crt, &serial);
    if (ret != 0) goto cleanup;
    
    // Set validity period
    ret = mbedtls_x509write_crt_set_validity(&crt, "20240101000000", "20341231235959");
    if (ret != 0) {
        lastError = "Failed to set certificate validity: " + String(ret);
        goto cleanup;
    }
    
    // Set subject and issuer (self-signed)
    ret = mbedtls_x509write_crt_set_subject_name(&crt, "CN=thermostat.local,O=ESP32,C=NL");
    if (ret != 0) {
        lastError = "Failed to set subject name: " + String(ret);
        goto cleanup;
    }
    
    ret = mbedtls_x509write_crt_set_issuer_name(&crt, "CN=thermostat.local,O=ESP32,C=NL");
    if (ret != 0) {
        lastError = "Failed to set issuer name: " + String(ret);
        goto cleanup;
    }
    
    // Set keys
    mbedtls_x509write_crt_set_subject_key(&crt, &key);
    mbedtls_x509write_crt_set_issuer_key(&crt, &key);
    
    // Set signature algorithm
    mbedtls_x509write_crt_set_md_alg(&crt, MBEDTLS_MD_SHA256);
    
    // Generate certificate
    ret = mbedtls_x509write_crt_pem(&crt, cert_buf, sizeof(cert_buf),
                                   mbedtls_ctr_drbg_random, &ctr_drbg);
    
    if (ret < 0) {
        lastError = "Failed to write certificate: " + String(ret);
        goto cleanup;
    }
    
    // Generate private key PEM
    ret = mbedtls_pk_write_key_pem(&key, key_buf, sizeof(key_buf));
    if (ret != 0) {
        lastError = "Failed to write private key: " + String(ret);
        goto cleanup;
    }
    
    // Convert to String objects
    certPEM = String((char*)cert_buf);
    keyPEM = String((char*)key_buf);
    
    Serial.println("X.509 certificate created successfully");
    
cleanup:
    mbedtls_x509write_crt_free(&crt);
    mbedtls_mpi_free(&serial);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    
    return ret;
}

bool SSLCertificate::loadStoredCertificate() {
    if (!preferences.isKey("cert_pem") || !preferences.isKey("key_pem") || !preferences.isKey("expiry")) {
        return false;
    }
    
    certificatePEM = preferences.getString("cert_pem", "");
    privateKeyPEM = preferences.getString("key_pem", "");
    expiryTime = preferences.getULong64("expiry", 0);
    
    return (!certificatePEM.isEmpty() && !privateKeyPEM.isEmpty() && expiryTime > 0);
}

bool SSLCertificate::saveCertificateToFlash() {
    if (certificatePEM.isEmpty() || privateKeyPEM.isEmpty()) {
        return false;
    }
    
    preferences.putString("cert_pem", certificatePEM);
    preferences.putString("key_pem", privateKeyPEM);
    preferences.putULong64("expiry", expiryTime);
    
    Serial.println("Certificate saved to flash memory");
    return true;
}

bool SSLCertificate::checkCertificateExpiry() const {
    if (expiryTime == 0) return false;
    
    unsigned long currentTime = millis();
    return currentTime < expiryTime;
}

// =============================================================================
// Public Interface Methods
// =============================================================================

String SSLCertificate::getCertificatePEM() const {
    return certificatePEM;
}

String SSLCertificate::getPrivateKeyPEM() const {
    return privateKeyPEM;
}

const char* SSLCertificate::getCertificateData() const {
    return certificatePEM.c_str();
}

const char* SSLCertificate::getPrivateKeyData() const {
    return privateKeyPEM.c_str();
}

bool SSLCertificate::isValid() const {
    return certificateValid && checkCertificateExpiry();
}

bool SSLCertificate::isExpiringSoon() const {
    if (expiryTime == 0) return true;
    
    unsigned long currentTime = millis();
    unsigned long timeUntilExpiry = expiryTime - currentTime;
    
    return timeUntilExpiry < (RENEWAL_THRESHOLD * 1000UL);
}

unsigned long SSLCertificate::getExpiryTime() const {
    return expiryTime;
}

unsigned long SSLCertificate::getDaysUntilExpiry() const {
    if (expiryTime == 0) return 0;
    
    unsigned long currentTime = millis();
    if (currentTime >= expiryTime) return 0;
    
    unsigned long timeUntilExpiry = expiryTime - currentTime;
    return timeUntilExpiry / (24UL * 60UL * 60UL * 1000UL);
}

void SSLCertificate::printCertificateInfo() const {
    Serial.println("=== SSL Certificate Info ===");
    Serial.printf("Valid: %s\n", certificateValid ? "Yes" : "No");
    Serial.printf("Days until expiry: %lu\n", getDaysUntilExpiry());
    Serial.printf("Certificate size: %d bytes\n", certificatePEM.length());
    Serial.printf("Private key size: %d bytes\n", privateKeyPEM.length());
    Serial.println("============================");
}

bool SSLCertificate::clearStoredCertificate() {
    preferences.remove("cert_pem");
    preferences.remove("key_pem");
    preferences.remove("expiry");
    
    certificatePEM = "";
    privateKeyPEM = "";
    expiryTime = 0;
    certificateValid = false;
    
    Serial.println("Stored certificate cleared");
    return true;
}

String SSLCertificate::getLastError() const {
    return lastError;
}