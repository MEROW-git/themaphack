# DNS and privacy audit

Scope: source and build inspection only. **VERIFIED** means checked against this checkout or the cited platform documentation; **UNVERIFIED** needs a device/network trace; **BROKEN** is a concrete source defect; **NOT IMPLEMENTED** is not wired into the active library.

## What executes

| Status | Finding |
| --- | --- |
| VERIFIED | `jni/Main.cpp` contains no call that configures Android Private DNS or intercepts native resolver APIs. `jni/Android.mk` does not compile or link a DNS-specific module. Its `netdb.h` include is not a DNS hook. |
| NOT IMPLEMENTED | All seven `Dns_*` functions in `jni/XYZ/Bypass.h:199-264` are legacy wrappers. The header is not included by the compiled sources, and there is no active registration for them. `BypassDNS = true` is therefore not a runtime setting in the built library. `Config.DnsString` in `jni/XYZ/ConfigName.h:301` has no read found in active source. |
| NOT IMPLEMENTED | No NextDNS hostname, account/profile ID, API client, or system-setting request occurs in the project source. The developer's requirement to use NextDNS is not enforced or technically supported by this library. |
| UNVERIFIED | Android or any other app on a particular phone may use a DNS provider, but there was no phone or traffic capture to inspect. |

## Legacy wrapper behavior if someone enabled it

| Status | Function | Source-level effect and problem |
| --- | --- | --- |
| BROKEN | `Dns_cctor` | Returns `true` without calling the original when `BypassDNS` is true; the original return type is unverified. |
| BROKEN | `Dns_GetHostByName_internal`, `Dns_GetHostByAddr_internal` | Replace multiple arguments with `ip.ml.youngjoygame.com` and put `:30220` on the first hostname string. A port suffix is not a DNS hostname. The presumed parameter types (`std::string`) are unverified for the target runtime. |
| BROKEN | `Dns_hostent_to_IPHostEntry` | Replaces all three inputs with one hostname, without validating that these fields are hostnames. |
| BROKEN | `Dns_GetHostByAddressFromString` | Changes an address lookup to `127.0.0.1`, which points to the device itself and can break connectivity. |
| BROKEN | `Dns_GetHostEntry`, `Dns_GetHostAddresses` | Substitute a hostname containing `:30220`, which may fail parsing or resolve the wrong endpoint. `Dns_GetHostAddresses` also passes the address of a C++ string to a `%s` logger, which expects a C string: undefined behavior if called. |
| BROKEN | `Dns_GetHostByName` | Calls the original once with a hardcoded host and then again with the input host, potentially making an extra query; its `%s` log has the same type error. |

All wrappers call uninitialized `oDns_*` pointers unless registered correctly. No registration exists in this build. The hardcoded name is `ip.ml.youngjoygame.com`; its current ownership, purpose, and DNS answers were not checked. The code does not mention NextDNS.

## Android Private DNS versus interception

Android's Private DNS is an operating-system resolver setting. A user can select a provider hostname in Android settings; it secures DNS questions and answers where the setting applies. It does not protect other traffic or make a modified application invisible. [Android Private DNS help](https://support.google.com/android/answer/9654714).

The legacy `Dns_*` wrappers would change arguments to certain application DNS calls if they were ever registered. They do not set the Android Private DNS provider, provide DNS-over-TLS, or intercept every native/library lookup. This difference matters: a hardcoded application hostname is not a NextDNS configuration. NextDNS's own help center describes Android's provider-hostname mode using a NextDNS DNS-over-TLS hostname; this repository contains no such configuration. [NextDNS help](https://help.nextdns.io/t/h7hll0w/private-dns-server-cannot-be-accessed).

Potential issues if the legacy wrappers were enabled: malformed hostnames can prevent connections; redirecting an address lookup to loopback can break or misroute traffic; extra lookups may disclose query names to the selected resolver. If Android Private DNS uses a third-party provider, that provider can receive DNS queries. Neither DNS method hides the destination IPs, application behavior, device/account identifiers, or server-side telemetry. These are source/platform implications, not a measured traffic result.

## Other network code

`jni/Tools/Login/Login.h` is **NOT IMPLEMENTED in the active build**. It contains an external HTTPS key endpoint and disables both `CURLOPT_SSL_VERIFYPEER` and `CURLOPT_SSL_VERIFYHOST` at lines 113-114 and 189-190. If reactivated, this would make TLS server authentication ineffective and risk exposing the submitted key/device identifier to an on-path attacker. It is separate from Android Private DNS. `jni/Main.cpp:173-207` contains an unused and **BROKEN** VPN detection helper; see [ANTIBAN_AUDIT.md](ANTIBAN_AUDIT.md).
