/* NOTE: GROUP ENROLLMENT
   ENROLLMENT GROUP
   All devices in the enrollment group must have a unique SAS token generated with
   the derived device key.
*/

// Fill in  your WiFi networks SSID and password
#define SECRET_SSID      "***"
#define SECRET_PASS      "***"

// Fill in the hostname and the ID scope of your Azure IoT DPS
#define SECRET_BROKER   "global.azure-devices-provisioning.net"
#define SECRET_ID_SCOPE "***"

// Fill in the device id (make sure that each device has a unique id)
#define SECRET_REGISTRATION_ID "dev-001"

// Device SAS token
#define SECRET_SAS_TOKEN "SharedAccessSignature sr=____registrations%2Fdev-001&sig=___=___&skn=registration"
