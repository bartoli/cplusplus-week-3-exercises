#include "main.h"

// ---- Supporting type member functions ---------------------------------------

bool TxInput::operator==(const TxInput& other) const {
    return prev_txid == other.prev_txid
        && prev_vout == other.prev_vout
        && script_sig == other.script_sig
        && sequence == other.sequence;
}

// Provided so UTXO can be stored in std::set. Order by (tx_id, vout_index, amount).
bool UTXO::operator<(const UTXO& other) const {
    if (tx_id != other.tx_id) return tx_id < other.tx_id;
    if (vout_index != other.vout_index) return vout_index < other.vout_index;
    return amount < other.amount;
}

bool UTXO::operator==(const UTXO& other) const {
    return tx_id == other.tx_id
        && vout_index == other.vout_index
        && amount == other.amount;
}

// ---- Exercises --------------------------------------------------------------

std::vector<uint8_t> CompactSizeEncoder::encode(uint64_t value) {
    // TODO: encode `value` into Bitcoin CompactSize bytes.
    //   value < 0xFD          -> single byte
    //   value <= 0xFFFF       -> 0xFD prefix + 2 little-endian bytes
    //   value <= 0xFFFFFFFF   -> 0xFE prefix + 4 little-endian bytes
    //   otherwise             -> 0xFF prefix + 8 little-endian bytes
    (void)value;
    return {};
}

std::optional<std::pair<uint64_t, std::size_t>> CompactSizeDecoder::decode(
    const std::vector<uint8_t>& data) {
    // TODO: decode a CompactSize integer from the front of `data`.
    // Return std::nullopt if `data` is empty or shorter than the prefix
    // demands. On success return {value, bytes_consumed}.
    (void)data;
    return std::nullopt;
}

// ---- TransactionData --------------------------------------------------------

TransactionData::TransactionData(uint32_t version, uint32_t lock_time)
    : version(version), lock_time(lock_time) {}

void TransactionData::add_input(const std::string& tx_id,
                                uint32_t vout_index,
                                const std::string& script_sig,
                                uint32_t sequence) {
    // TODO: build a TxInput from the arguments and append it to `inputs`.
    (void)tx_id;
    (void)vout_index;
    (void)script_sig;
    (void)sequence;
}

void TransactionData::add_output(uint64_t value_satoshi,
                                 const std::string& script_pubkey) {
    // TODO: append value_satoshi and script_pubkey to `outputs`.
    (void)value_satoshi;
    (void)script_pubkey;
}

std::vector<TxInput> TransactionData::get_input_details() const {
    // TODO: iterate over `inputs` and return a copy of each entry.
    return {};
}

std::pair<uint64_t, std::size_t> TransactionData::summarize_outputs(
    uint64_t min_value) const {
    // TODO: walk `outputs` with a while loop, summing values that are >= min_value.
    //       Skip (continue) outputs below min_value.
    //       Break out of the loop as soon as the running total exceeds
    //       1,000,000,000 satoshis. Return {total_satoshi, valid_outputs_count}.
    (void)min_value;
    return {0, 0};
}

void TransactionData::update_metadata(
    const std::unordered_map<std::string, std::string>& new_data) {
    // TODO: merge `new_data` into `metadata`, overwriting existing keys.
    (void)new_data;
}

std::string TransactionData::get_metadata_value(
    const std::string& key, const std::string& default_value) const {
    // TODO: return metadata[key] if present, otherwise `default_value`.
    (void)key;
    return default_value;
}

std::tuple<uint32_t, std::size_t, std::size_t, uint32_t>
TransactionData::get_transaction_header() const {
    // TODO: return the version, inputs.size(), outputs.size(), and lock_time.
    return {0, 0, 0, 0};
}

void TransactionData::set_transaction_header(uint32_t new_version,
                                             std::size_t num_inputs,
                                             std::size_t num_outputs,
                                             uint32_t new_lock_time) {
    // TODO: update `version` and `lock_time`. Ignore num_inputs/num_outputs.
    //       Hint: practice structured bindings or std::tie with `_` placeholders.
    (void)new_version;
    (void)num_inputs;
    (void)num_outputs;
    (void)new_lock_time;
}

// ---- UTXOSet ----------------------------------------------------------------

void UTXOSet::add_utxo(const std::string& tx_id,
                       uint32_t vout_index,
                       uint64_t amount) {
    // TODO: construct a UTXO and insert it into `utxos`.
    (void)tx_id;
    (void)vout_index;
    (void)amount;
}

bool UTXOSet::remove_utxo(const std::string& tx_id,
                          uint32_t vout_index,
                          uint64_t amount) {
    // TODO: remove the matching UTXO from `utxos`. Return true if it was present, false otherwise.
    (void)tx_id;
    (void)vout_index;
    (void)amount;
    return false;
}

uint64_t UTXOSet::get_balance() const {
    // TODO: sum `amount` across every UTXO in `utxos`.
    return 0;
}

std::set<UTXO> UTXOSet::find_sufficient_utxos(uint64_t target_amount) const {
    // TODO: walk `utxos` in order, collecting them until their summed amount
    //       reaches `target_amount`. Return the collected set, or an empty
    //       set if the target cannot be met.
    (void)target_amount;
    return {};
}

std::size_t UTXOSet::get_total_utxo_count() const {
    // TODO: return the number of UTXOs in `utxos`.
    return 0;
}

bool UTXOSet::is_subset_of(const UTXOSet& other) const {
    // TODO: return true iff every UTXO in `utxos` is also in `other.utxos`.
    //       Hint: see std::includes in <algorithm>.
    (void)other;
    return false;
}

UTXOSet UTXOSet::combine_utxos(const UTXOSet& other) const {
    // TODO: return a new UTXOSet whose `utxos` is the union of this set's UTXOs and `other.utxos`.
    //       Hint: std::set_union.
    (void)other;
    return {};
}

UTXOSet UTXOSet::find_common_utxos(const UTXOSet& other) const {
    // TODO: return a new UTXOSet whose `utxos` is the intersection of this set's UTXOs and `other.utxos`. 
    //       Hint: std::set_intersection.
    (void)other;
    return {};
}

// ---- Block header generator -------------------------------------------------

void generate_block_headers(
    const std::string& prev_block_hash,
    const std::string& merkle_root,
    uint32_t timestamp,
    uint32_t bits,
    uint32_t start_nonce,
    uint32_t max_attempts,
    const std::function<bool(const BlockHeader&)>& on_header) {
    // TODO: starting from `start_nonce`, build a BlockHeader for each nonce and call on_header(header). Stop early if on_header returns false.
    //       Otherwise stop after `max_attempts` attempts.
    (void)prev_block_hash;
    (void)merkle_root;
    (void)timestamp;
    (void)bits;
    (void)start_nonce;
    (void)max_attempts;
    (void)on_header;
}
