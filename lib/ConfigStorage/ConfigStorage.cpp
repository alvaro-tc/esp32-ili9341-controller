/**
 * ConfigStorage Library Implementation - Simple Configuration Storage
 * 
 * Implementación súper simple para guardar configuraciones
 */

#include "ConfigStorage.h"

// Constructor
ConfigStorage::ConfigStorage() {
    activeProfile = 0;
    
    // Inicializar configuración por defecto
    resetCurrentConfig();
}

// ========== FUNCIONES BÁSICAS ==========

bool ConfigStorage::begin() {
    // Inicializar Preferences
    bool success = preferences.begin("config", false);
    
    if (success) {
        // Cargar perfil activo guardado (si existe)
        activeProfile = preferences.getUChar("active", 0);
        
        // Asegurar que el perfil activo esté en rango válido
        if (activeProfile >= MAX_PROFILES) {
            activeProfile = 0;
            preferences.putUChar("active", activeProfile);
        }
        
        // Cargar configuración del perfil activo
        bool configLoaded = loadCurrentConfig();
        
        // Si no se pudo cargar, usar valores por defecto
        if (!configLoaded) {
            resetCurrentConfig();
            saveCurrentConfig(); // Guardar los valores por defecto
        }
    }
    
    return success;
}

void ConfigStorage::end() {
    preferences.end();
    Serial.println("ConfigStorage cerrado");
}

// ========== GESTIÓN DE PERFILES ==========

bool ConfigStorage::setActiveProfile(uint8_t profile) {
    if (profile >= MAX_PROFILES) {
        return false;
    }
    
    // Solo guardar configuración actual si estamos cambiando de perfil
    if (activeProfile != profile) {
        // Guardar configuración del perfil actual
        saveCurrentConfig();
        
        // Cambiar perfil activo
        activeProfile = profile;
        
        // Guardar INMEDIATAMENTE el nuevo perfil activo en flash
        bool activeSaved = preferences.putUChar("active", activeProfile);
        
        if (!activeSaved) {
            // Si no se pudo guardar, revertir el cambio
            activeProfile = (activeProfile == 0) ? profile : activeProfile;
            return false;
        }
        
        // Cargar configuración del nuevo perfil
        bool configLoaded = loadCurrentConfig();
        
        if (!configLoaded) {
            // Si no existe configuración para este perfil, crear una por defecto
            resetCurrentConfig();
            saveCurrentConfig();
        }
    }
    
    return true;
}

uint8_t ConfigStorage::getActiveProfile() {
    return activeProfile;
}

// ========== GUARDAR/CARGAR CONFIGURACIÓN ==========

bool ConfigStorage::saveCurrentConfig() {
    return saveConfigToProfile(activeProfile);
}

bool ConfigStorage::loadCurrentConfig() {
    return loadConfigFromProfile(activeProfile);
}

bool ConfigStorage::saveConfigToProfile(uint8_t profile) {
    if (profile >= MAX_PROFILES) {
        return false;
    }
    
    // Generar claves para este perfil
    String valuesKey = getProfileKey(profile);
    String addressKey = getAddressKey(profile);
    
    // Guardar los 15 valores uint8_t como array
    size_t written = preferences.putBytes(valuesKey.c_str(), currentConfig.values, CONFIG_VALUES_COUNT);
    
    // Guardar la dirección uint64_t
    bool addressSaved = preferences.putULong64(addressKey.c_str(), currentConfig.address);
    
    bool success = (written == CONFIG_VALUES_COUNT) && addressSaved;
    
    if (success) {
        Serial.print("✅ Configuración guardada en perfil ");
        Serial.print(profile);
        Serial.print(" ("); Serial.print(written); Serial.println(" bytes)");
        
        // Mostrar algunos valores guardados para debug
        Serial.print("🔍 Valores guardados: [");
        Serial.print(currentConfig.values[0]); Serial.print(",");
        Serial.print(currentConfig.values[12]); Serial.print(",");
        Serial.print(currentConfig.values[14]); Serial.println("]");
    } else {
        Serial.print("❌ Error guardando en perfil ");
        Serial.print(profile);
        Serial.print(" - Escritos: "); Serial.print(written);
        Serial.print("/"); Serial.print(CONFIG_VALUES_COUNT);
        Serial.print(" Addr: "); Serial.println(addressSaved ? "OK" : "FAIL");
    }
    
    return success;
}

bool ConfigStorage::loadConfigFromProfile(uint8_t profile) {
    if (profile >= MAX_PROFILES) {
        return false;
    }
    
    // Generar claves para este perfil
    String valuesKey = getProfileKey(profile);
    String addressKey = getAddressKey(profile);
    
    // Intentar cargar los 15 valores uint8_t
    size_t bytesRead = preferences.getBytes(valuesKey.c_str(), currentConfig.values, CONFIG_VALUES_COUNT);
    
    // Cargar la dirección uint64_t (valor por defecto si no existe)
    currentConfig.address = preferences.getULong64(addressKey.c_str(), 0xE8E8F0F0E1LL);
    
    bool success = (bytesRead == CONFIG_VALUES_COUNT);
    
    // Si no se leyeron 15 valores, puede ser un perfil con 14 valores (versión anterior)
    if (!success && bytesRead == 14) {
        // Migración automática: establecer valor por defecto para intensidad (índice 14)
        currentConfig.values[14] = 1;  // Intensidad por defecto = 1
        
        // Guardar el perfil migrado inmediatamente
        saveConfigToProfile(profile);
        success = true;
    }
    else if (!success || bytesRead == 0) {
        // Si no existe configuración o está vacía, retornar false
        // El caller debe usar resetCurrentConfig() y saveCurrentConfig()
        return false;
    }
    
    return success;
}

// ========== ACCESO A DATOS ==========

void ConfigStorage::setValue(uint8_t index, uint8_t value) {
    if (index < CONFIG_VALUES_COUNT) {
        currentConfig.values[index] = value;
        
        Serial.print("Valor [");
        Serial.print(index);
        Serial.print("] = ");
        Serial.println(value);
    }
}

uint8_t ConfigStorage::getValue(uint8_t index) {
    if (index < CONFIG_VALUES_COUNT) {
        return currentConfig.values[index];
    }
    return 0;
}

void ConfigStorage::setAddress(uint64_t address) {
    currentConfig.address = address;
    
    Serial.print("Dirección = 0x");
    Serial.println((uint32_t)(address >> 32), HEX);
    Serial.println((uint32_t)(address & 0xFFFFFFFF), HEX);
}

uint64_t ConfigStorage::getAddress() {
    return currentConfig.address;
}

void ConfigStorage::setConfig(const ConfigProfile& config) {
    currentConfig = config;
    Serial.println("Configuración completa actualizada");
}

ConfigProfile ConfigStorage::getConfig() {
    return currentConfig;
}

// ========== FUNCIONES DE UTILIDAD ==========

void ConfigStorage::resetProfile(uint8_t profile) {
    if (profile >= MAX_PROFILES) {
        return;
    }
    
    // Crear configuración por defecto
    ConfigProfile defaultConfig;
    
    // Valores por defecto para los 14 uint8_t
    for (uint8_t i = 0; i < CONFIG_VALUES_COUNT; i++) {
        defaultConfig.values[i] = 128; // Valor medio (50%)
    }
    
    // Dirección por defecto
    defaultConfig.address = 0xE8E8F0F0E1LL;
    
    // Guardar temporalmente la config actual
    ConfigProfile temp = currentConfig;
    
    // Aplicar config por defecto y guardar
    currentConfig = defaultConfig;
    saveConfigToProfile(profile);
    
    // Restaurar config actual
    currentConfig = temp;
    
    Serial.print("✅ Perfil ");
    Serial.print(profile);
    Serial.println(" reseteado a valores por defecto");
}

void ConfigStorage::resetCurrentConfig() {
    Serial.println("🔄 Reseteando configuración actual...");
    
    // Valores específicos por defecto para cada función
    // LÍMITES DE VELOCIDAD (índices 0, 1, 2)
    currentConfig.values[0] = 80;   // Velocidad baja
    currentConfig.values[1] = 160;  // Velocidad media
    currentConfig.values[2] = 255;  // Velocidad alta
    
    // LÍMITES DE GIRO (índices 3, 4, 5)  
    currentConfig.values[3] = 50;   // Giro suave
    currentConfig.values[4] = 128;  // Giro normal
    currentConfig.values[5] = 200;  // Giro agresivo
    
    // LÍMITES DE BOOST (índices 6, 7, 8)
    currentConfig.values[6] = 100;  // Boost bajo
    currentConfig.values[7] = 180;  // Boost medio
    currentConfig.values[8] = 255;  // Boost máximo
    
    // LÍMITES ADICIONALES (índices 9, 10, 11)
    currentConfig.values[9] = 120;  // Extra 1
    currentConfig.values[10] = 140; // Extra 2
    currentConfig.values[11] = 160; // Extra 3
    
    // LÍMITE DE BRILLO (índice 12)
    currentConfig.values[12] = 200; // Brillo por defecto
    
    // CONFIGURACIÓN ADICIONAL (índice 13)
    currentConfig.values[13] = 128; // Config extra
    
    // CONFIGURACIÓN DE INTENSIDAD (índice 14)
    currentConfig.values[14] = 1;   // Intensidad por defecto
    
    // Dirección por defecto NRF24L01
    currentConfig.address = 0xE8E8F0F0E1LL;
    
    Serial.println("✅ Configuración reseteada con valores por defecto");
}



bool ConfigStorage::isProfileEmpty(uint8_t profile) {
    if (profile >= MAX_PROFILES) {
        return true;
    }
    
    String valuesKey = getProfileKey(profile);
    
    // Verificar si existe la clave de valores
    return !preferences.isKey(valuesKey.c_str());
}

void ConfigStorage::printCurrentConfig() {
    Serial.println("=== Configuración Actual ===");
    Serial.print("Perfil activo: ");
    Serial.println(activeProfile);
    
    Serial.print("Valores: [");
    for (uint8_t i = 0; i < CONFIG_VALUES_COUNT; i++) {
        Serial.print(currentConfig.values[i]);
        if (i < CONFIG_VALUES_COUNT - 1) Serial.print(", ");
    }
    Serial.println("]");
    
    Serial.print("Dirección: 0x");
    Serial.println((unsigned long)currentConfig.address, HEX);
    Serial.println("===========================");
}

void ConfigStorage::printProfile(uint8_t profile) {
    if (profile >= MAX_PROFILES) {
        Serial.println("Perfil inválido");
        return;
    }
    
    Serial.print("=== Perfil ");
    Serial.print(profile);
    
    if (isProfileEmpty(profile)) {
        Serial.println(" (VACÍO) ===");
        return;
    }
    
    Serial.println(" ===");
    
    // Cargar temporalmente el perfil
    ConfigProfile temp = currentConfig;
    loadConfigFromProfile(profile);
    
    Serial.print("Valores: [");
    for (uint8_t i = 0; i < CONFIG_VALUES_COUNT; i++) {
        Serial.print(currentConfig.values[i]);
        if (i < CONFIG_VALUES_COUNT - 1) Serial.print(", ");
    }
    Serial.println("]");
    
    Serial.print("Dirección: 0x");
    Serial.println((unsigned long)currentConfig.address, HEX);
    Serial.println("==================");
    
    // Restaurar configuración original
    currentConfig = temp;
}

// ========== FUNCIONES RÁPIDAS ==========

bool ConfigStorage::quickSave(uint8_t profile, uint8_t values[CONFIG_VALUES_COUNT], uint64_t address) {
    if (profile >= MAX_PROFILES) {
        return false;
    }
    
    // Crear configuración temporal
    ConfigProfile tempConfig;
    
    // Copiar valores
    for (uint8_t i = 0; i < CONFIG_VALUES_COUNT; i++) {
        tempConfig.values[i] = values[i];
    }
    tempConfig.address = address;
    
    // Guardar configuración actual
    ConfigProfile savedConfig = currentConfig;
    
    // Aplicar configuración temporal y guardar
    currentConfig = tempConfig;
    bool success = saveConfigToProfile(profile);
    
    // Restaurar configuración original
    currentConfig = savedConfig;
    
    return success;
}

bool ConfigStorage::quickLoad(uint8_t profile, uint8_t values[CONFIG_VALUES_COUNT], uint64_t* address) {
    if (profile >= MAX_PROFILES || address == nullptr) {
        return false;
    }
    
    // Guardar configuración actual
    ConfigProfile savedConfig = currentConfig;
    
    // Cargar desde el perfil
    bool success = loadConfigFromProfile(profile);
    
    if (success) {
        // Copiar valores al array proporcionado
        for (uint8_t i = 0; i < CONFIG_VALUES_COUNT; i++) {
            values[i] = currentConfig.values[i];
        }
        *address = currentConfig.address;
    }
    
    // Restaurar configuración original
    currentConfig = savedConfig;
    
    return success;
}

// ========== FUNCIONES ESPECÍFICAS PARA DATOS ORGANIZADOS ==========

// LÍMITES DE VELOCIDAD (índices 0, 1, 2)
void ConfigStorage::setSpeedLimits(uint8_t limit1, uint8_t limit2, uint8_t limit3) {
    currentConfig.values[0] = limit1;
    currentConfig.values[1] = limit2;
    currentConfig.values[2] = limit3;
    
    Serial.println("✅ Límites de velocidad configurados:");
    Serial.print("  Límite 1: "); Serial.println(limit1);
    Serial.print("  Límite 2: "); Serial.println(limit2);
    Serial.print("  Límite 3: "); Serial.println(limit3);
}

void ConfigStorage::setSpeedLimit(uint8_t index, uint8_t value) {
    if (index < 3) {
        currentConfig.values[index] = value;
        Serial.print("Límite velocidad ["); Serial.print(index); 
        Serial.print("] = "); Serial.println(value);
    }
}

uint8_t ConfigStorage::getSpeedLimit(uint8_t index) {
    if (index < 3) {
        return currentConfig.values[index];
    }
    return 0; 
}

void ConfigStorage::getSpeedLimits(uint8_t* limit1, uint8_t* limit2, uint8_t* limit3) {
    if (limit1) *limit1 = currentConfig.values[0];
    if (limit2) *limit2 = currentConfig.values[1];
    if (limit3) *limit3 = currentConfig.values[2];
}

// LÍMITES DE GIRO (índices 3, 4, 5)
void ConfigStorage::setTurnLimits(uint8_t limit1, uint8_t limit2, uint8_t limit3) {
    currentConfig.values[3] = limit1;
    currentConfig.values[4] = limit2;
    currentConfig.values[5] = limit3;
    
    Serial.println("✅ Límites de giro configurados:");
    Serial.print("  Límite 1: "); Serial.println(limit1);
    Serial.print("  Límite 2: "); Serial.println(limit2);
    Serial.print("  Límite 3: "); Serial.println(limit3);
}

void ConfigStorage::setTurnLimit(uint8_t index, uint8_t value) {
    if (index < 3) {
        currentConfig.values[3 + index] = value;
        Serial.print("Límite giro ["); Serial.print(index); 
        Serial.print("] = "); Serial.println(value);
    }
}

uint8_t ConfigStorage::getTurnLimit(uint8_t index) {
    if (index < 3) {
        return currentConfig.values[3 + index];
    }
    return 0;
}

void ConfigStorage::getTurnLimits(uint8_t* limit1, uint8_t* limit2, uint8_t* limit3) {
    if (limit1) *limit1 = currentConfig.values[3];
    if (limit2) *limit2 = currentConfig.values[4];
    if (limit3) *limit3 = currentConfig.values[5];
}

// LÍMITES DE BOOST (índices 6, 7, 8)
void ConfigStorage::setBoostLimits(uint8_t limit1, uint8_t limit2, uint8_t limit3) {
    currentConfig.values[6] = limit1;
    currentConfig.values[7] = limit2;
    currentConfig.values[8] = limit3;
    
    Serial.println("✅ Límites de boost configurados:");
    Serial.print("  Límite 1: "); Serial.println(limit1);
    Serial.print("  Límite 2: "); Serial.println(limit2);
    Serial.print("  Límite 3: "); Serial.println(limit3);
}

void ConfigStorage::setBoostLimit(uint8_t index, uint8_t value) {
    if (index < 3) {
        currentConfig.values[6 + index] = value;
        Serial.print("Límite boost ["); Serial.print(index); 
        Serial.print("] = "); Serial.println(value);
    }
}

uint8_t ConfigStorage::getBoostLimit(uint8_t index) {
    if (index < 3) {
        return currentConfig.values[6 + index];
    }
    return 0;
}

void ConfigStorage::getBoostLimits(uint8_t* limit1, uint8_t* limit2, uint8_t* limit3) {
    if (limit1) *limit1 = currentConfig.values[6];
    if (limit2) *limit2 = currentConfig.values[7];
    if (limit3) *limit3 = currentConfig.values[8];
}

// LÍMITES ADICIONALES (índices 9, 10, 11)
void ConfigStorage::setExtraLimits(uint8_t limit1, uint8_t limit2, uint8_t limit3) {
    currentConfig.values[9] = limit1;
    currentConfig.values[10] = limit2;
    currentConfig.values[11] = limit3;
    
    Serial.println("✅ Límites adicionales configurados:");
    Serial.print("  Límite 1: "); Serial.println(limit1);
    Serial.print("  Límite 2: "); Serial.println(limit2);
    Serial.print("  Límite 3: "); Serial.println(limit3);
}

void ConfigStorage::setExtraLimit(uint8_t index, uint8_t value) {
    if (index < 3) { 
        currentConfig.values[9 + index] = value;
        Serial.print("Límite extra ["); Serial.print(index); 
        Serial.print("] = "); Serial.println(value);
    }
}

uint8_t ConfigStorage::getExtraLimit(uint8_t index) {
    if (index < 3) {
        return currentConfig.values[9 + index];
    }
    return 0;
}

void ConfigStorage::getExtraLimits(uint8_t* limit1, uint8_t* limit2, uint8_t* limit3) {
    if (limit1) *limit1 = currentConfig.values[9];
    if (limit2) *limit2 = currentConfig.values[10];
    if (limit3) *limit3 = currentConfig.values[11];
}

// LÍMITE DE BRILLO (índice 12)
void ConfigStorage::setBrightnessLimit(uint8_t brightness) {
    currentConfig.values[12] = brightness;
    Serial.print("✅ Brillo configurado: "); Serial.println(brightness);
}

uint8_t ConfigStorage::getBrightnessLimit() {
    return currentConfig.values[12];
}

// CONFIGURACIÓN ADICIONAL (índice 13)
void ConfigStorage::setExtraConfig(uint8_t config) {
    currentConfig.values[13] = config;
    Serial.print("✅ Config extra: "); Serial.println(config);
}

uint8_t ConfigStorage::getExtraConfig() {
    return currentConfig.values[13];
}

// DIRECCIÓN NRF24L01
void ConfigStorage::setNRFAddress(uint64_t address) {
    currentConfig.address = address;
    Serial.print("✅ Dirección NRF24L01: 0x");
    Serial.println((unsigned long)address, HEX);
}

uint64_t ConfigStorage::getNRFAddress() {
    return currentConfig.address;
}

// INFORMACIÓN DE CONFIGURACIÓN ACTIVA
uint8_t ConfigStorage::getActiveProfileNumber() {
    return activeProfile;
}

String ConfigStorage::getActiveProfileName() {
    return "Perfil " + String(activeProfile);
}

void ConfigStorage::printActiveConfig() {
    Serial.println("=== CONFIGURACIÓN ACTIVA ===");
    Serial.print("Perfil: "); Serial.println(getActiveProfileName());
    
    Serial.print("Velocidad: [");
    for (int i = 0; i < 3; i++) {
        Serial.print(getSpeedLimit(i));
        if (i < 2) Serial.print(", ");
    }
    Serial.println("]");
    
    Serial.print("Giro: [");
    for (int i = 0; i < 3; i++) {
        Serial.print(getTurnLimit(i));
        if (i < 2) Serial.print(", ");
    }
    Serial.println("]");
    
    Serial.print("Boost: [");
    for (int i = 0; i < 3; i++) {
        Serial.print(getBoostLimit(i));
        if (i < 2) Serial.print(", ");
    }
    Serial.println("]");
    
    Serial.print("Extra: [");
    for (int i = 0; i < 3; i++) {
        Serial.print(getExtraLimit(i));
        if (i < 2) Serial.print(", ");
    }
    Serial.println("]");
    
    Serial.print("Brillo: "); Serial.println(getBrightnessLimit());
    Serial.print("Config: "); Serial.println(getExtraConfig());
    Serial.print("NRF Addr: 0x"); Serial.println((unsigned long)getNRFAddress(), HEX);
    Serial.println("============================");
}

// FUNCIONES DE CARGA Y VERIFICACIÓN
bool ConfigStorage::isConfigurationLoaded() {
    return !isProfileEmpty(activeProfile);
}

void ConfigStorage::reloadActiveConfig() {
    loadCurrentConfig();
    Serial.print("✅ Configuración recargada desde perfil ");
    Serial.println(activeProfile);
}

// ========== FUNCIONES PRIVADAS ==========

String ConfigStorage::getProfileKey(uint8_t profile) {
    return "p" + String(profile) + "v"; // "p0v", "p1v", etc.
}

String ConfigStorage::getAddressKey(uint8_t profile) {
    return "p" + String(profile) + "a"; // "p0a", "p1a", etc.
}

// CONFIGURACIÓN DE INTENSIDAD (índice 14)
void ConfigStorage::setIntensity(uint8_t intensity) {
    // Validar que esté en rango 1-4
    if (intensity < 1) intensity = 1;
    if (intensity > 4) intensity = 4;
    
    // Guardar en el índice 14 (pero restar 1 para que sea 0-3 internamente)
    if (CONFIG_VALUES_COUNT > 14) {
        currentConfig.values[14] = intensity;
        Serial.print("✅ Intensidad configurada: "); Serial.println(intensity);
    }
}

uint8_t ConfigStorage::getIntensityLimit() {
    if (CONFIG_VALUES_COUNT > 14) {
        uint8_t intensity = currentConfig.values[14];
        // Si es 0, devolver 1 como valor por defecto
        if (intensity == 0) intensity = 1;
        // Validar rango 1-4
        if (intensity > 4) intensity = 4;
        return intensity;
    }
    return 1; // Valor por defecto
}

// ========== FUNCIONES DE MANTENIMIENTO ==========

void ConfigStorage::clearAllProfiles() {
    Serial.println("🗑️  Limpiando todos los perfiles...");
    
    for (uint8_t i = 0; i < MAX_PROFILES; i++) {
        String valuesKey = getProfileKey(i);
        String addressKey = getAddressKey(i);
        
        preferences.remove(valuesKey.c_str());
        preferences.remove(addressKey.c_str());
        
        Serial.print("✅ Perfil ");
        Serial.print(i);
        Serial.println(" limpiado");
    }
    
    // Limpiar perfil activo también
    preferences.remove("active");
    
    // Resetear configuración actual
    resetCurrentConfig();
    activeProfile = 0;
    
    Serial.println("🔄 Todos los perfiles han sido limpiados. Reinicie el dispositivo.");
}

bool ConfigStorage::repairProfile(uint8_t profile) {
    if (profile >= MAX_PROFILES) {
        return false;
    }
    
    Serial.print("🔧 Reparando perfil ");
    Serial.println(profile);
    
    // Guardar configuración actual
    ConfigProfile backup = currentConfig;
    
    // Resetear a valores por defecto
    resetCurrentConfig();
    
    // Guardar el perfil reparado
    bool success = saveConfigToProfile(profile);
    
    // Restaurar configuración actual
    currentConfig = backup;
    
    if (success) {
        Serial.print("✅ Perfil ");
        Serial.print(profile);
        Serial.println(" reparado correctamente");
    } else {
        Serial.print("❌ Error reparando perfil ");
        Serial.println(profile);
    }
    
    return success;
}