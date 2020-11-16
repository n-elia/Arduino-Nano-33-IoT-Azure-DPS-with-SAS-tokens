'''
   Azure IoT Device Provisioning Service SAS token generation tool
   This Python script allows you to rapidly generate SAS tokens for
   Individual Enrollments or Group Enrollments.

   Author: Nicola Elia
   GNU General Public License v3.0
'''

from base64 import b64encode, b64decode
from hashlib import sha256
from time import time
from urllib.parse import quote_plus, urlencode
from hmac import HMAC


def generate_sas_token(uri, key, policy_name, expiry=3600):
    # Time to live is set such that we can specify a duration (expiry) in [s] since
    # the token creation. Then, the UNIX timestamp of the expiry date will be created.
    ttl = time() + expiry
    sign_key = "%s\n%d" % ((quote_plus(uri)), int(ttl))
    signature = b64encode(
        HMAC(b64decode(key), sign_key.encode('utf-8'), sha256).digest())

    rawtoken = {
        'sr':  uri,
        'sig': signature,
        'se': str(int(ttl))
    }

    if policy_name is not None:
        rawtoken['skn'] = policy_name

    return 'SharedAccessSignature ' + urlencode(rawtoken)


def generate_device_key(registrationId, masterKey):
    '''Creates a device derived key by hashing the device unique registrationId with the DPS key.'''
    deviceKey = b64encode(HMAC(b64decode(masterKey),
                               registrationId.encode('utf-8'),
                               sha256)
                          .digest())
    return deviceKey.decode('UTF-8')


if __name__ == "__main__":
    # ------------- SELECT MODE -------------
    select = 2
    '''
    Select 1: create a SAS token for a device in an Enrollment Group
    Select 2: create a SAS token for a device in an Individual Enrollment
    '''

    if select == 1:
        # Device derived key generation for DPS Enrollment Groups
        # ------------- INSERT DATA -------------
        dps_scopeId = '***'
        device_registration_id = '***'
        dps_key = '***'
        # ----------- INSERT OPTIONS ------------
        # Expiry time in [s] from token creation
        expiry = 365*12*30*24*60*60  # [s]
        # ---------------------------------------
        
        # Default policy for device registration
        policy = 'registration'
        # uri: [dps_scope_id]/registrations/[device_registration_id]
        uri = dps_scopeId + '/registrations/' + device_registration_id
        
        # Device derived key generation
        device_derived_key = generate_device_key(device_registration_id, dps_key)
        print('Device "' + device_registration_id + '" derived key: ' + device_derived_key)

        # Device SAS token generation
        device_sas_token = generate_sas_token(uri, device_derived_key, policy, expiry)
        print('Device SAS token: ' + device_sas_token)


    if select == 2:
        # Individual Enrollment SAS token generation
        # ------------- INSERT DATA -------------
        dps_scopeId = '***'
        device_registration_id = '***' # Coincides with Individual Enrollment ID
        dps_key = '***'
        # ----------- INSERT OPTIONS ------------
        # Expiry time in [s] from token creation
        expiry = 365*12*30*24*60*60
        # ---------------------------------------

        # uri: [dps_scope_id]/registrations/[device_registration_id]
        uri = dps_scopeId + '/registrations/' + device_registration_id
        # Default policy for device registration
        policy = 'registration'
        device_sas_token = generate_sas_token(uri, dps_key, policy, expiry)
        print('Device/Individual Enrollment SAS token: ' + device_sas_token)
