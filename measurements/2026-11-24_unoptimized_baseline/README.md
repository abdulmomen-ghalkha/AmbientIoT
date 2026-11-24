# Unoptimized baseline power profile (24 Nov 2026)

PPK2 profile of the unoptimized node (guide section XI). Report item D3, unoptimized column.

## Report figures

| Figure | File | Average | Max | Charge |
|---|---|---|---|---|
| 10 s window | [unoptimized_10s-window.png](unoptimized_10s-window.png) | **11.76 mA** | 53.42 mA | 117.55 mC |
| 1 s window | [unoptimized_1s-window.png](unoptimized_1s-window.png) | **11.76 mA** | 47.98 mA | 11.76 mC |

Both windows come from one recording (run 2 below), taken in steady state about 30 s after power-up with the phone connected. The Power Profiler session is saved as `ppk2-20261124T161531_session.ppk2`, so the windows can be re-exported.

## Conditions

| Item | Value |
|---|---|
| Setup | PPK2 in Ampere Meter mode, in series between the bench supply and the Nucleo 3V3 pin, common GND ([photo](../../docs/photos/2026-11-24_ppk2-profiling-setup.jpeg)) |
| Supply | 3.3 V bench supply, 100 mA limit (guide section XI) |
| Nucleo | SB14 and SB9 open (ST-LINK off), USB unplugged |
| Load on the 3.3 V rail | STM32L412KB, SHT31 breakout, soil-moisture sensor (switched by PB1), XIAO nRF52840 |
| STM32 firmware | Busy-wait loop, cycle about 2 s (K8). Flashed build sends `ADC=… Temp=… C Hum=… %`; **not yet committed** (K14) |
| XIAO firmware | `unoptimized_BLE_full_connection` at commit `988f479` (connected Nordic UART, +4 dBm, one notification per reading) |
| BLE state | Phone **connected** in nRF Connect, notifications enabled ([screenshot at 18:02](../../docs/nrf-connect/2026-11-24_unoptimized_ppk2-power.jpeg)) |
| Soil probe | In dry air, reads `ADC=0.00` |
| Extra breadboard LED | Removed before these runs. Nucleo LD2 (power) and the XIAO LED are still on |
| Sampling | 100 kS/s |

## Files

| File | Content |
|---|---|
| `unoptimized_10s-window.png`, `unoptimized_1s-window.png` | **Report figures** (run 2) |
| `ppk2-20261124T161531_session.ppk2` | Power Profiler session of run 2 |
| `ppk-20261124T161551_steady-29s_final.csv.gz` | Run 2: 29.0 s steady-state export (31–60 s after power-up) |
| `ppk-20261124T160020_steady-29s.csv.gz` | Run 1: 29.0 s steady-state export |
| `ppk-20261124T160027_steady-29s-window.png` | Run 1: 29 s window screenshot (statistics cut off; superseded) |
| `ppk-20261124T155429_boot-12s.csv.gz` | Run 1: 12.5 s capture including the start-up spike |

The CSVs are gzipped (lossless) because the raw exports are 33–81 MB; raw `.csv` files are git-ignored. Columns: `Timestamp(ms), Current(uA), D0-D7`. To unpack: `gunzip -k <file>.csv.gz`.

## Results

Computed from the CSVs (every sample).

| Metric | Run 2, steady 29 s (**final**) | Run 1, steady 29 s | Run 1, start-up 12.5 s | Guide expectation |
|---|---|---|---|---|
| Average current | **11.76 mA** | 11.82 mA | 12.61 mA | – |
| Floor (1st percentile) | **9.52 mA** | 9.61 mA | 9.61 mA | about 10 mA |
| Median | 11.04 mA | 11.13 mA | 11.23 mA | – |
| Typical burst peak (events above 20 mA) | **32.1 mA**, every 4.2 ms | 32.3 mA, every 4.2 ms | 32.8 mA | 25–30 mA (BLE) |
| Large peaks (events above 35 mA) | median 42.5 mA, 109 events, mostly 300–330 ms apart | median 42.8 mA, 118 events | median 46.9 mA, 195 events | – |
| Maximum | 53.42 mA | 53.70 mA | 56.65 mA (at 0.24 s, start-up) | – |
| 100 ms averages | 11.50–11.91 mA | 11.76–12.20 mA | 11.66–13.60 mA | – |

Average power at 3.3 V: 11.76 mA × 3.3 V ≈ **38.8 mW**. The two steady runs agree within 0.5 %, so the baseline is reproducible.

**Reading the profile:** the floor of about 9.5 mA is the STM32 busy-waiting plus the XIAO with its radio stack and LEDs. On top of that, the radio adds short bursts of about 32 mA every 4.2 ms and occasional larger peaks of 40–53 mA. The 100 ms averages are flat, so the 2 s sensing cycle is hidden under the radio activity: nothing ever sleeps. This matches the guide's description of the unoptimized profile (about 10 mA floor with BLE spikes), with peaks a little higher than the guide's 25–30 mA. These are the targets for the optimization: STOP2 sleep lowers the floor, burst beaconing removes the continuous connection bursts.

## Caveat

The STM32 firmware that produced these figures is not in git yet (K14). Commit it before tagging `v1-unoptimized`. If the code changes before the tag (for example the 5 s cycle from K8), repeat this run so the figures match the tagged code.
