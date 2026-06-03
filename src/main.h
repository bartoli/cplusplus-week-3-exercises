#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

// ---- Transaction input ------------------------------------------------------

struct TxInput {
    std::string prev_txid;
    uint32_t prev_vout = 0;
    std::string script_sig;
    uint32_t sequence = 0xFFFFFFFFu;

    bool operator==(const TxInput& other) const;
};

// ---- UTXO -------------------------------------------------------------------

struct UTXO {
    std::string tx_id;
    uint32_t vout_index = 0;
    uint64_t amount = 0;

    bool operator<(const UTXO& other) const;
    bool operator==(const UTXO& other) const;
};

// ---- Block header -----------------------------------------------------------

struct BlockHeader {
    uint32_t version = 2;
    std::string prev_block_hash;
    std::string merkle_root;
    uint32_t timestamp = 0;
    uint32_t bits = 0;
    uint32_t nonce = 0;
};

// ---- CompactSize encoder ----------------------------------------------------

class CompactSizeEncoder {
public:
    // Encode `value` into Bitcoin CompactSize bytes.
    //
    //   value < 0xFD          -> single byte
    //   value <= 0xFFFF       -> 0xFD + 2 little-endian bytes
    //   value <= 0xFFFFFFFF   -> 0xFE + 4 little-endian bytes
    //   otherwise             -> 0xFF + 8 little-endian bytes
    std::vector<uint8_t> encode(uint64_t value);
};

// ---- CompactSize decoder ----------------------------------------------------

class CompactSizeDecoder {
public:
    // Decode a CompactSize integer from the front of `data`.
    // Returns {value, bytes_consumed} on success, or std::nullopt
    // if `data` is empty or too short for the indicated prefix.
    std::optional<std::pair<uint64_t, std::size_t>> decode(
        const std::vector<uint8_t>& data);
};

// ---- Transaction data -------------------------------------------------------

class TransactionData {
public:
    uint32_t version = 1;
    std::vector<TxInput> inputs;
    std::vector<std::pair<uint64_t, std::string>> outputs;  // (value_satoshi, script_pubkey)
    uint32_t lock_time = 0;
    std::unordered_map<std::string, std::string> metadata;

    explicit TransactionData(uint32_t version = 1, uint32_t lock_time = 0);

    void add_input(const std::string& tx_id,
                   uint32_t vout_index,
                   const std::string& script_sig,
                   uint32_t sequence = 0xFFFFFFFFu);

    void add_output(uint64_t value_satoshi, const std::string& script_pubkey);

    // Return a copy of each input record.
    std::vector<TxInput> get_input_details() const;

    // Sum outputs whose value is >= min_value. Stops early (break) if the
    // running total exceeds 1,000,000,000 satoshis. Returns
    // {total_satoshi, valid_outputs_count}.
    std::pair<uint64_t, std::size_t> summarize_outputs(uint64_t min_value = 0) const;

    // Merge new_data into metadata (overwriting existing keys).
    void update_metadata(const std::unordered_map<std::string, std::string>& new_data);

    // Return metadata[key] or default_value if absent.
    std::string get_metadata_value(const std::string& key,
                                   const std::string& default_value = "") const;

    // Return {version, inputs.size(), outputs.size(), lock_time}.
    std::tuple<uint32_t, std::size_t, std::size_t, uint32_t> get_transaction_header() const;

    // Update version and lock_time. num_inputs and num_outputs are accepted
    // but intentionally ignored (use structured bindings / std::tie to skip).
    void set_transaction_header(uint32_t version,
                                std::size_t num_inputs,
                                std::size_t num_outputs,
                                uint32_t lock_time);
};

// ---- UTXO set ---------------------------------------------------------------

class UTXOSet {
public:
    std::set<UTXO> utxos;

    void add_utxo(const std::string& tx_id, uint32_t vout_index, uint64_t amount);

    // Returns true if the UTXO was present and removed.
    bool remove_utxo(const std::string& tx_id, uint32_t vout_index, uint64_t amount);

    uint64_t get_balance() const;

    // Greedily pick UTXOs (in set order) until their summed amount >= target.
    // Returns the chosen subset, or an empty set if the target cannot be met.
    std::set<UTXO> find_sufficient_utxos(uint64_t target_amount) const;

    std::size_t get_total_utxo_count() const;

    bool is_subset_of(const UTXOSet& other) const;

    UTXOSet combine_utxos(const UTXOSet& other) const;

    UTXOSet find_common_utxos(const UTXOSet& other) const;
};

// ---- Block header generator -------------------------------------------------

// Repeatedly build a BlockHeader (incrementing nonce from start_nonce) and
// invoke on_header(header) for each attempt. Stops when on_header returns
// false or after max_attempts iterations.
void generate_block_headers(
    const std::string& prev_block_hash,
    const std::string& merkle_root,
    uint32_t timestamp,
    uint32_t bits,
    uint32_t start_nonce,
    uint32_t max_attempts,
    const std::function<bool(const BlockHeader&)>& on_header);
