# `litime_mppt` component for ESPHome

This implements basic modbus-over-BLE to solar charge controllers (maximum power
point trackers, or MPPT). This interface is used by certain controllers sold
under several brand names, including LiTime and HQST.

This is based on https://github.com/mavenius/litime_mppt_esphome/, converted to
an external component instead of an include. This makes it easier to integrate
and gets proper config validation.

## Supported device(s)

Tested with:

 - LiTime BT-LTMPPT4860
 - LiTime BT-LTMPPT2430
 - HQST M4860N


### Disclaimer

This is not an official library endorsed by the device manufacturer. LiTime and all other trademarks in this repo are the property of their respective owners and their use herein does not imply any sponsorship or endorsement. Further, all code contained herein is provided "as is" without guarantees of performance or suitability; use it at your own risk.
