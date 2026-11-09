# Basic Garbled Circuit

### Steps

#### Garbling
1. Generate two labels for each wire, representing 0 and 1.
2. For each gate:
- Enumerate all possible input combinations.
- Use HKDF-SHA256 to derive a 256-bit AES key from the input labels.
- Encrypt the corresponding output label concatenated with a 128-bit zero using the key.
- Store the resulting ciphertext in the garbled table.

#### Transferring
1. Save circuit and gabrler's input labels into .bin files, transfer to evaluator.
2. Evaluator get labels corresponding to its input via Oblivious Transfer. (todo)

#### Evaluating
(todo)

### File Structure

#### 1. garbled_circuit bin 

Header

| Field | Type | Size |
|---|---|---:|
| Magic (`CHII`) | char[4] | 4 bytes |
| Version | uint32 | 4 bytes |
| num_wires | uint64 | 8 bytes |
| num_gates | uint64 | 8 bytes |
| num_input_parties | uint32 | 4 bytes |
| input_sizes | uint64[] | num_input_parties × 8 bytes |
| num_output_parties | uint32 | 4 bytes |
| output_sizes | uint64[] | num_output_parties × 8 bytes |

Gate

| Field | Type | Size |
|---|---|---:|
| num_inputs | uint32 | 4 bytes |
| inputs | uint64[] | num_inputs × 8 bytes |
| output | uint64 | 8 bytes |
| num_rows | uint32 | 4 bytes |
| garbled_table | GarbledRow[] | num_rows × 32 bytes |

GarbledRow

| Field | Type | Size |
|---|---|---:|
| ciphertext | uint8[16] | 16 bytes |
| tag | uint8[16] | 16 bytes |

#### 2. input_labels.bin

This file stores the selected labels of garbler's inputs.

Header

| Field | Type | Size |
|---|---|---:|
| Magic (`KAWA`) | char[4] | 4 bytes |
| Version | uint32 | 4 bytes |
| party_id | uint32 | 4 bytes |
| num_labels | uint64 | 8 bytes |

Label

| Field | Type | Size |
|---|---|---:|
| label | uint8[16] | 16 bytes |
