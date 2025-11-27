## System
AArch64 GNU/Linux Debian, 8 cores, 8 GB DDR4 RAM  

- GCC: Debian 15.2.0-9  
- OpenMP: GNU libgomp 4.5  

ALL PROGRAMS WERE TESTED ON THE SAME DATASET.

## results

| Program   | Mean [s]       | Min [s] | Max [s] | Relative       |
|:--------- |--------------:|--------:|--------:|---------------|
| GNU du    | 10.008 ± 1.339 | 9.368  | 12.403 | 5.49 ± 0.78   |
| UDU       | 1.824 ± 0.086  | 1.729  | 1.928  | 1.00           |
