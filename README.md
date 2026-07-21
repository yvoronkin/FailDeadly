# FailDeadly
### Fail Secure. Fail Deadly

FailDeadly is a USB hardware device designed to ensure the physical security of the server. It uses Hall sensors, along with magnets installed on the server lid, to detect any tampering attempt. To make it work, FailDeadly integrates with infrastructure and becomes the main or one of the sources of trust to pass server remote attestation.

FailDeadly stores secrets such as the sign key in SRAM as long as the MCU has power. In case of a tampering attempt when the server lid is moved, device power management detects a change in the magnetic field and cuts off the MCU power supply. FailDeadly destroy in memory secret in one second or less after the tamper event is detected. This time delay allows the MCU to notify the host system about the tamper event through message, SysRq, or hardware reset event.

To be able to keep secrets and reduce the number of maintenance events during power outages, FailDeadly has an Ionister to keep the SRAM powered.

### ** _Warning:_ FailDeadly achives it goals by valuing confidentiality more than availability. Be aware.**

Features:

* Physical tamper detection
* In-memory secret storage
* Power cut off on bare logic - no way to cancel 
* Secret destruction in case of tampering event
* SysRq or Reset pin could be used to reset the host system.
* On-board ionistor connector to keep secrets through power outages
* HashiCorp Vault integration
* On-device encrypt/decrypt, sign/verity operations.
* On-device key generation

Use cases:

* Hardening server physical security
* Remote server attestation
* Burn-out LUKS decryption keys

![](/staff/img/FailDeadly.png)

### TODO:
- [x] Hardware design v1.0 rev 001 
- [ ] Firmware 
- [ ] Software 
- [ ] HashiCorp Vault login plugin 

